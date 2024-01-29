#pragma once

class ImgFuncWithSampling : public ImgFuncBase
{
public:
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
