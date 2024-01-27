#pragma once

class ImgFunc_shdc02 : public ImgFuncBase
{
public:
	const char* getName() const;
	const char* getSummary() const;

	bool run(const cv::Mat& SrcImg, cv::Mat& dstImg);

private:
	/// Determine the threshold th1 for making mask.
	double getTh1FromBluredBlackHatResult(
		const cv::Mat& bluredBhatImg,
		const cv::Rect& binROI
	);

	/// Make a mask to separate lines and background.
	bool makeMaskImage(const cv::Mat& srcImg, cv::Mat& mask);

	/// Sample pixels.
	std::vector<LumSample> sampleImage(const cv::Mat_<uchar>& image);

	//
	//	For DEBUG
	//

	/// Dump approximation result visually. (For DEBUG.)
	void dumpAppxImg(
		const cv::Mat srcImg,
		const std::vector<double>& cflist,
		const char* const caption,
		const char* const dstDir
	);

	/// Plot sample points. (For DEBUG.)
	void plotSamples(
		const cv::Mat_<uchar>& srcImg,
		const std::vector<LumSample>& samples,
		const char* const caption,
		const char* const dstDir
	);
};
