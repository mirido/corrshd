#pragma once

class ImgFunc_shdc01 : public ImgFuncBase
{
	const char* getName() const;

	bool run(const cv::Mat& SrcImg, cv::Mat& dstImg);

};

