#pragma once

class ImgFunc_shdc04 : public ImgFuncBase
{
	ImgFunc_whitening02 m_imgFunc_Whiteing;
	ImgFunc_uniform m_imgFunc_uniform;

public:
	ImgFunc_shdc04(Param& param);

	const char* getName() const;
	const char* getSummary() const;

	bool run(const cv::Mat& SrcImg, cv::Mat& dstImg);

};
