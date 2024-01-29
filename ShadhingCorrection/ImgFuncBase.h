#pragma once

class ImgFuncBase : public IImgFunc
{
	static unsigned long m_imgDumpCnt;		// Thread safety is ignored.

public:
	ImgFuncBase();

	//
	//	For DEBUG
	//

	/// Reset image dump count.
	void resetImgDumpCnt();

	/// Dump intermediate image. (For DEBUG.)
	void dumpImg(const cv::Mat& image, const char* const caption, const char* const dstDir);

};

// [DBGSW] Directory to save intermediate images
// TODO: Make it variable by command line arguments.
#define DBG_IMG_DIR		"C:\\usr2\\debug\\"

//
//	Utily
//

cv::Rect get_bin_ROI(const cv::Size& imgSz);

cv::Size get_bin_kernel_size(const cv::Size& imgSz);

cv::Mat get_bin_kernel(const cv::Size& imgSz);
