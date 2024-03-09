#pragma once

#include "ImgFunc_shdcWithUniform.h"

class ImgFunc_shdc03 : public ImgFunc_shdcWithUniform
{
	ImgFunc_whitening01 m_whitening01;

public:
	ImgFunc_shdc03(ParamPtr pParam);

	const char* getName() const;
	const char* getSummary() const;

private:
	bool whitening(
		const cv::Mat& srcImg,
		cv::Mat& dstImg,
		cv::Mat& maskToKeepDrawLine
	);

};
