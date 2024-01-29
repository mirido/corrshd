#pragma once

class ImgFunc_shdc03 : public ImgFuncBase
{
	ImgFunc_whitening02 m_imgFunc_Whiteing;
	ImgFunc_uniform m_imgFunc_uniform;

public:
	ImgFunc_shdc03();

	const char* getName() const;
	const char* getSummary() const;

	bool run(const cv::Mat& SrcImg, cv::Mat& dstImg);

};
