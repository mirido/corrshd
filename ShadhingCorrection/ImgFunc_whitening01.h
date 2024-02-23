#pragma once

#include "WithFinalInversion.h"
#include "WithMaskToKeepDrawLine.h"

class ImgFunc_whitening01
	: public ImgFuncBase, public WithFinalInversion, public WithMaskToKeepDrawLine
{
	cv::Mat m_customKernel;

public:
	ImgFunc_whitening01(Param& param);

	const char* getName() const;
	const char* getSummary() const;

	bool run(const cv::Mat& SrcImg, cv::Mat& dstImg);

	/// Method to keep the same behavior as before refactoring on shdc02.
	void setCustomKernel(const cv::Mat& kernel);

};
