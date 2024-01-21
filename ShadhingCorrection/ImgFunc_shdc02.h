#pragma once

class ImgFunc_shdc02 : public ImgFuncBase
{
public:
	const char* getName() const;

	bool run(const cv::Mat& SrcImg, cv::Mat& dstImg);

private:
	double getTh1FromBluredBlackHatResult(
		const cv::Mat& bluredBhatImg,
		const cv::Rect& binROI
	);

};
