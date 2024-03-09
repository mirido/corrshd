#pragma once

#include "ImgFuncWithSampling.h"
#include "WithMaskToKeepDrawLine.h"

class ImgFunc_uniform
	: public ImgFuncWithSampling, public WithMaskToKeepDrawLine
{
public:
	ImgFunc_uniform(ParamPtr pParam);

	const char* getName() const;
	const char* getSummary() const;

	bool run(const cv::Mat& SrcImg, cv::Mat& dstImg);

private:
	/// Sample pixels on drawing line. 
	std::vector<LumSample> sampleDrawLine(
		const cv::Mat_<uchar>& invImage,
		const cv::Mat_<uchar>& maskForDLChg
	);

};
