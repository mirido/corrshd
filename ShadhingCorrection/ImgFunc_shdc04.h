#pragma once

#include "ImgFunc_shdcWithUniform.h"

class ImgFunc_shdc04 : public ImgFunc_shdcWithUniform
{
	ImgFunc_whitening02 m_whitening02;

public:
	ImgFunc_shdc04(Param& param);

	const char* getName() const;
	const char* getSummary() const;

private:
	bool whitening(
		const cv::Mat& srcImg,
		cv::Mat& dstImg,
		cv::Mat& maskToKeepDrawLine
	);

};
