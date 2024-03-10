#pragma once

class WithMaskToKeepDrawLine
{
	cv::Mat m_maskToKeepDrawLine;
	double m_thToKeepDrawLine;
	bool m_bMakeMaskToKeepDrawLine;

public:
	WithMaskToKeepDrawLine();

	void setFlagToMakeMaskToKeepDrawLine(const bool bMake);

	bool getFlagToMakeMaskToKeepDrawLine() const;

protected:
	void updateMaskToKeepDrawLine(
		const cv::Mat& srcImg,
		const double ratioOfSmpROIToImgSz,
		const cv::InputArray globalMask
	);

	void releaseMaskToKeepDrawLine();

public:
	const cv::Mat& getMaskToKeepDrawLine() const;

	double getThToKeepDrawLine() const;

private:
	void makeMaskToKeepDrawLine(
		const cv::Mat& srcImg,
		const double ratioOfSmpROIToImgSz,
		const cv::InputArray globalMask
	);

	/// Get binarization threshold with Otsu.
	double getThWithOtsu(
		const cv::Mat& bluredBhatImg,
		const cv::Rect& binROI,
		const cv::InputArray globalMask
	);

};
