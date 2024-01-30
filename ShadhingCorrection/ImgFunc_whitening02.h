#pragma once

#include "ImgFuncWithSampling.h"
#include "WithFinalInversion.h"
#include "WithMaskNearZeroToZero.h"

class ImgFunc_whitening02 : public ImgFuncWithSampling, public WithFinalInversion, public WithMaskNearZeroToZero
{
public:
	const char* getName() const;
	const char* getSummary() const;

	bool run(const cv::Mat& SrcImg, cv::Mat& dstImg);

private:
	/// Sample pixels.
	std::vector<LumSample> sampleImage(const cv::Mat_<uchar>& image);

};
