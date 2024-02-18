#pragma once

class ImgFuncBase : public IImgFunc
{
public:
	struct Param
	{
		std::shared_ptr<bool> m_pbDump;
		std::shared_ptr<unsigned long> m_pImgCnt;
		std::shared_ptr<std::string> m_pDbgImgDir;

		std::shared_ptr<std::vector<std::string> > m_pWndNameList;
		std::shared_ptr<std::vector<std::string> > m_pImgFileList;

		Param();
	};

private:
	Param m_param;

public:
	ImgFuncBase(Param& param);

	//
	//	For DEBUG
	//

	/// Reset image dump count.
	void resetImgDumpCnt();

	/// Dump intermediate image. (For DEBUG.)
	void dumpImg(const cv::Mat& image, const char* const caption);

	/// Clean up intermediage image.
	void cleanup();

};

// [CONF] Default directory to save intermediate images
#define DBG_IMG_DIR		"C:\\usr2\\debug\\"

#include "bin_kernel.h"
