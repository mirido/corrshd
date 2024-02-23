#include "stdafx.h"
#include "IImgFunc.h"
#include "ImgFuncBase.h"

#include "pathutil.h"

// [CONF] Default image file extension.
#define DEFAULT_IMG_EXT		".bmp"

// [CONF] Stride between windows.
#define STRIDE_WND_X		(1920 / 20)
#define STRIDE_WND_Y		20

// [CONF] ROI size for determine binarization threshold (ratio)
#define BIN_ROI_RATIO		0.8

ImgFuncBase::Param::Param()
	: m_pbDump(new bool(false))
	, m_pImgCnt(new unsigned long(0))
	, m_pDbgImgDir(new std::string(DBG_IMG_DIR))
	, m_pWndNameList(new std::vector<std::string>)
	, m_pImgFileList(new std::vector<std::string>)
	, m_pRatioOfSmpROIToImgSz(new double(BIN_ROI_RATIO))
	, m_pMaskToAvoidFgObj(new cv::Mat)
{
	/*pass*/
}

ImgFuncBase::ImgFuncBase(Param& param)
	: m_param(param)
{
	/*pass*/
}

//
//	For DEBUG
//

/// Reset image dump count.
void ImgFuncBase::resetImgDumpCnt()
{
	*m_param.m_pImgCnt = C_ULONG(0);
	cleanup();
}

/// Dump intermediate image. (For DEBUG.)
void ImgFuncBase::dumpImg(const cv::Mat& image, const char* const caption)
{
	if (!*m_param.m_pbDump) {
		return;
	}

	(*m_param.m_pImgCnt)++;

	cv::String numStr = std::to_string(*m_param.m_pImgCnt);
	if (numStr.length() < 2) {
		numStr = cv::String("0") + numStr;
	}

	// Display the image on the screen.
	cv::String cap = numStr + cv::String("_") + caption;
	cv::imshow(cap, image);
	const int wnd_x = (*m_param.m_pImgCnt) * STRIDE_WND_X;
	const int wnd_y = (*m_param.m_pImgCnt) * STRIDE_WND_Y;
	cv::moveWindow(cap, wnd_x, wnd_y);
	m_param.m_pWndNameList->push_back(cap);

	// Save image to specified directory.
	const char* const dstDir = m_param.m_pDbgImgDir->c_str();
	if (dstDir != NULL) {
		// Make directory path string.
		cv::String dir(dstDir);
		if (strchr("\\/", CHAR_TO_INT(dir.back())) == NULL) {
			dir += cv::String("/");
		}

		// Replace space to '_' in cap.
		std::string cap2;
		for (auto it = cap.begin(); it != cap.end(); it++) {
			int c = CHAR_TO_INT(*it);
			if (isspace(c)) {
				c = '_';
			}
			cap2.push_back(C_UCHAR(c));
		}
		cap = cap2;

		// Decide filename.
		// If cap have no extension, add DEFAULT_IMG_EXT as extension.
		std::string dummy, filenameMajor, ext;
		parse_file_name(cap.c_str(), dummy, filenameMajor, ext);
		cv::String filename(cap);
		if (ext.empty()) {
			filename += cv::String(DEFAULT_IMG_EXT);
		}

		// Save image.
		const std::string fpath = dir + filename;
		cv::imwrite(fpath, image);
		m_param.m_pImgFileList->push_back(fpath);
	}
}

/// Clean up intermediage image.
void ImgFuncBase::cleanup()
{
	// Destroy intermediate image windows.
	auto pWndNameList = m_param.m_pWndNameList;		// Alias
	for (auto it = pWndNameList->begin(); it != pWndNameList->end(); it++) {
		cv::destroyWindow(*it);
	}
	pWndNameList->clear();

	// Delete intermediate image files.
	auto pImgFileList = m_param.m_pImgFileList;		// Alias
	for (auto it = pImgFileList->begin(); it != pImgFileList->end(); it++) {
		std::remove(it->c_str());
	}
	pImgFileList->clear();
}
