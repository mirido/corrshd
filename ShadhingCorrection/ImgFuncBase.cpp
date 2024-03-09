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
	: m_bDump(false)
	, m_imgCnt(C_ULONG(0))
	, m_dbgImgDir(DBG_IMG_DIR)
	, m_ratioOfSmpROIToImgSz(C_DBL(BIN_ROI_RATIO))
{
	/*pass*/
}

ImgFuncBase::ImgFuncBase(ParamPtr pParam)
	: m_pParam(pParam)
{
	/*pass*/
}

//
//	For DEBUG
//

/// Reset image dump count.
void ImgFuncBase::resetImgDumpCnt()
{
	m_pParam->m_imgCnt = C_ULONG(0);
	cleanup();
}

/// Dump intermediate image. (For DEBUG.)
void ImgFuncBase::dumpImg(const cv::Mat& image, const char* const caption)
{
	if (!m_pParam->m_bDump) {
		return;
	}

	(m_pParam->m_imgCnt)++;

	cv::String numStr = std::to_string(m_pParam->m_imgCnt);
	if (numStr.length() < 2) {
		numStr = cv::String("0") + numStr;
	}

	// Display the image on the screen.
	cv::String cap = numStr + cv::String("_") + caption;
	cv::imshow(cap, image);
	const int wnd_x = (m_pParam->m_imgCnt) * STRIDE_WND_X;
	const int wnd_y = (m_pParam->m_imgCnt) * STRIDE_WND_Y;
	cv::moveWindow(cap, wnd_x, wnd_y);
	auto& wndNameList = m_pParam->m_wndNameList;		// Alias
	wndNameList.push_back(cap);

	// Save image to specified directory.
	auto& dbgImgDir = m_pParam->m_dbgImgDir;		// Alias
	const char* const dstDir = dbgImgDir.c_str();
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
		auto& imgFileList = m_pParam->m_imgFileList;		// Alias
		imgFileList.push_back(fpath);
	}
}

/// Clean up intermediage image.
void ImgFuncBase::cleanup()
{
	// Destroy intermediate image windows.
	auto& wndNameList = m_pParam->m_wndNameList;		// Alias
	for (auto it = wndNameList.begin(); it != wndNameList.end(); it++) {
		try {
			cv::destroyWindow(*it);
		}
		catch (cv::Exception& e) {
			const char* const msg = e.what();
			cout << "Warning: " << msg << endl;
		}
	}
	wndNameList.clear();

	// Delete intermediate image files.
	auto& imgFileList = m_pParam->m_imgFileList;		// Alias
	for (auto it = imgFileList.begin(); it != imgFileList.end(); it++) {
		std::remove(it->c_str());
	}
	imgFileList.clear();
}
