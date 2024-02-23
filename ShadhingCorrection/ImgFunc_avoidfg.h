#pragma once

#include "ImgFunc_whitening02.h"

class ImgFunc_avoidfg : public ImgFuncBase
{
	ImgFunc_whitening02 m_whitening02;

public:
	ImgFunc_avoidfg(Param& param);

	const char* getName() const;
	const char* getSummary() const;

	bool run(const cv::Mat& SrcImg, cv::Mat& dstImg);

private:
	void whitenYUVImg(const cv::Mat& YUVSrcImg, cv::Mat& modifiedYUVImg);

	void dumpYUVImgAsBGR(const cv::Mat& YUVImg, const char* const caption);

};
