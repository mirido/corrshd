#pragma once

class ImgFunc_shdc02 : public ImgFuncBase
{
public:
	const char* getName() const;

	bool run(const cv::Mat& SrcImg, cv::Mat& dstImg);

private:
	/// Determine the threshold th1 for making mask.
	double getTh1FromBluredBlackHatResult(
		const cv::Mat& bluredBhatImg,
		const cv::Rect& binROI
	);

	/// Make a mask to separate lines and background.
	bool makeMaskImage(const cv::Mat& srcImg, cv::Mat& mask);

	/// Sample unmasked pixels.
	std::vector<LumSample> sampleImage(
		const cv::Mat_<uchar>& image, const cv::Mat_<uchar>& mask, const size_t nsamples);

};
