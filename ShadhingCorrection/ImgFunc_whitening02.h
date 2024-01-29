#pragma once

class ImgFunc_whitening02 : public ImgFuncWithSampling
{
	bool m_bNormalLumGradation;

public:
	ImgFunc_whitening02();

	void setLumGradiationToNormal(const bool bNormal);

	const char* getName() const;
	const char* getSummary() const;

	bool run(const cv::Mat& SrcImg, cv::Mat& dstImg);

private:
	/// Sample pixels.
	std::vector<LumSample> sampleImage(const cv::Mat_<uchar>& image);

};
