#include "stdafx.h"
#include "IImgFunc.h"
#include "ImgFuncBase.h"

#include "../libimaging/geometryutil.h"
#include "pathutil.h"

// [CONF] Default image file extension.
#define DEFAULT_IMG_EXT		".bmp"

// [CONF] ROI size for determine binarization threshold (ratio)
#define BIN_ROI_RATIO		0.8

// [CONF] Kernel size for determine binarization threshold (ratio)
#define BIN_KERNEL_RATIO	0.025

//
//	For DEBUG
//

unsigned long ImgFuncBase::m_imgDumpCnt = C_ULONG(0);

ImgFuncBase::ImgFuncBase()
{
	/*pass*/
}

/// Reset image dump count.
void ImgFuncBase::resetImgDumpCnt()
{
	m_imgDumpCnt = C_ULONG(0);
}

/// Dump intermediate image. (For DEBUG.)
void ImgFuncBase::dumpImg(const cv::Mat& image, const char* const caption, const char* const dstDir)
{
#ifdef NDEBUG
	(void)(image, caption, dstDir);
#else
	m_imgDumpCnt++;

	cv::String numStr = std::to_string(m_imgDumpCnt);
	if (numStr.length() < 2) {
		numStr = cv::String("0") + numStr;
	}

	// Display the image on the screen.
	const cv::String cap = numStr + cv::String("_") + caption;
	cv::imshow(cap, image);

	// Save image to specified directory.
	if (dstDir != NULL) {
		// Make directory path string.
		cv::String dir(dstDir);
		if (strchr("\\/", CHAR_TO_INT(dir.back())) == NULL) {
			dir += cv::String("/");
		}

		// Decide filename.
		// If cap have no extension, add DEFAULT_IMG_EXT as extension.
		std::string dummy, filenameMajor, ext;
		parse_file_name(cap.c_str(), dummy, filenameMajor, ext);
		cv::String filename(cap);
		if (ext.empty()) {
			filename += cv::String(DEFAULT_IMG_EXT);
		}

		// Save image.
		cv::imwrite(dir + filename, image);
	}
#endif
}

//
//	Utily
//

cv::Rect get_bin_ROI(const cv::Size& imgSz)
{
	return get_scaled_rect_from_size(imgSz, BIN_ROI_RATIO);
}

cv::Size get_bin_kernel_size(const cv::Size& imgSz)
{
	const int ageImgSz = (imgSz.width, imgSz.height) / 2;
	const int knsz0 = (int)std::round(C_DBL(ageImgSz) * BIN_KERNEL_RATIO);
	const int knszOdd = std::max(3, 2 * ((knsz0 + 1) / 2) + 1);
	return cv::Size(knszOdd, knszOdd);
}

cv::Mat get_bin_kernel(const cv::Size& imgSz)
{
	const cv::Size kernelSz = get_bin_kernel_size(imgSz);
	return cv::getStructuringElement(cv::MORPH_ELLIPSE, kernelSz);
}
