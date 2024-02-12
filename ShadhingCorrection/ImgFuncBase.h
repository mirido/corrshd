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

#include "bin_kernel.h"
