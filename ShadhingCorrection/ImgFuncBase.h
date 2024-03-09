#pragma once

class ImgFuncBase : public IImgFunc
{
public:
	struct Param
	{
		bool m_bDump;
		unsigned long m_imgCnt;
		std::string m_dbgImgDir;

		std::vector<std::string> m_wndNameList;
		std::vector<std::string> m_imgFileList;

		// Global data for image processing
		double m_ratioOfSmpROIToImgSz;		// Ratio of sampling ROI to image size.
		cv::Mat m_maskToAvoidFgObj;			// Mask to avoid foreground objects.

		Param();
	};

	using ParamPtr = std::shared_ptr<Param>;

protected:
	ParamPtr m_pParam;

public:
	ImgFuncBase(ParamPtr pParam);

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
