#pragma once

#include "WithFinalInversion.h"

class ImgFunc_whitening01 : public ImgFuncBase, public WithFinalInversion
{
public:
	const char* getName() const;
	const char* getSummary() const;

	bool run(const cv::Mat& SrcImg, cv::Mat& dstImg);

};
