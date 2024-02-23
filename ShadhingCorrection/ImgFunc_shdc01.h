#pragma once

#include "ImgFunc_whitening01.h"

class ImgFunc_shdc01 : public ImgFuncBase
{
	ImgFunc_whitening01 m_whitening01;

public:
	ImgFunc_shdc01(Param& param);

	const char* getName() const;
	const char* getSummary() const;

	bool run(const cv::Mat& SrcImg, cv::Mat& dstImg);

};
