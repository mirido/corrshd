#pragma once

#include "ImgFunc_whitening01.h"
#include "ImgFunc_whitening02.h"

class ImgFunc_shdc02 : public ImgFuncWithSampling
{
	ImgFunc_whitening02 m_whitening02;
	ImgFunc_whitening01 m_whitening01;

public:
	ImgFunc_shdc02(ParamPtr pParam);

	const char* getName() const;
	const char* getSummary() const;

	bool run(const cv::Mat& SrcImg, cv::Mat& dstImg);

private:
	/// Sample pixels on drawing line. 
	std::vector<LumSample> sampleDrawLine(
		const cv::Mat_<uchar>& invImage, const cv::Mat_<uchar>& maskForDLChg, const size_t nsamples);

};
