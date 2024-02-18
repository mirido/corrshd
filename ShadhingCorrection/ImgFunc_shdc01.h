#pragma once

class ImgFunc_shdc01 : public ImgFuncBase
{
public:
	ImgFunc_shdc01(Param& param);

	const char* getName() const;
	const char* getSummary() const;

	bool run(const cv::Mat& SrcImg, cv::Mat& dstImg);

};

