#pragma once

#include "ImgFuncWithSampling.h"
#include "WithFinalInversion.h"
#include "WithMaskToKeepDrawLine.h"
#include "WithStdWhiteImg.h"

class ImgFunc_whitening02
	: public ImgFuncWithSampling, public WithFinalInversion, public WithMaskToKeepDrawLine
	, public WithStdWhiteImg
{
	size_t m_lastSizeOfSamplesOnBG;

public:
	ImgFunc_whitening02(Param& param);

	const char* getName() const;
	const char* getSummary() const;

	bool run(const cv::Mat& SrcImg, cv::Mat& dstImg);

	/// Method to keep the same behavior as before refactoring on shdc02.
	size_t getLastSizeOfSamplesOnBG() const;

private:
	/// Sample pixels.
	std::vector<LumSample> sampleImage(const cv::Mat_<uchar>& image);

};
