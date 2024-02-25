#pragma once

class WithMaskToKeepDrawLine
{
protected:
	cv::Mat m_maskToKeepDrawLine;
	double m_thToKeepDrawLine;
	bool m_bNeedMaskToKeepDrawLine;

public:
	WithMaskToKeepDrawLine();

	void needMaskToKeepDrawLine(const bool bNeed);

	void makeMaskToKeepDrawLine(
		const cv::Mat& srcImg,
		const double ratioOfSmpROIToImgSz,
		const cv::InputArray globalMask
	);

	const cv::Mat& getMaskToKeepDrawLine() const;

private:
	/// Get binarization threshold with Otsu.
	double getThWithOtsu(
		const cv::Mat& bluredBhatImg,
		const cv::Rect& binROI,
		const cv::InputArray globalMask
	);

};
