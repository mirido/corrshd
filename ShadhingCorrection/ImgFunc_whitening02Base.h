#pragma once

#include "ImgFuncWithSampling.h"
#include "WithFinalInversion.h"
#include "WithMaskToKeepDrawLine.h"
#include "WithStdWhiteImg.h"

class ImgFunc_whitening02Base
	: public ImgFuncWithSampling, public WithFinalInversion, public WithMaskToKeepDrawLine
	, public WithStdWhiteImg
{
public:
	ImgFunc_whitening02Base(Param& param);

	bool run(const cv::Mat& SrcImg, cv::Mat& dstImg);

private:
	/// Sample pixels.
	virtual std::vector<LumSample> sampleImage(const cv::Mat_<uchar>& image) = 0;

};
