#pragma once

class ImgFunc_whitening01 : public ImgFuncBase
{
	bool m_bNormalLumGradation;

public:
	ImgFunc_whitening01();

	void setLumGradiationToNormal(const bool bNormal);

	const char* getName() const;
	const char* getSummary() const;

	bool run(const cv::Mat& SrcImg, cv::Mat& dstImg);

};
