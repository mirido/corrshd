#pragma once

class ImgFunc_uniform : public ImgFuncWithSampling
{
	bool m_bNormalLumGradation;

public:
	ImgFunc_uniform();

	void setLumGradiationToNormal(const bool bNormal);

	const char* getName() const;
	const char* getSummary() const;

	bool run(const cv::Mat& SrcImg, cv::Mat& dstImg);

private:
	/// Get binarization threshold with Otsu.
	double getTh1WithOtsu(
		const cv::Mat& bluredBhatImg,
		const cv::Rect& binROI
	);

	/// Make mask image for drawing line change.
	void makeMaskImage(const cv::Mat& srcImg, cv::Mat& mask);

	/// Sample pixels on drawing line. 
	std::vector<LumSample> sampleDrawLine(
		const cv::Mat_<uchar>& invImage,
		const cv::Mat_<uchar>& maskForDLChg
	);

};
