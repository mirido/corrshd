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

protected:
	void makeMaskToKeepDrawLine(const cv::Mat& srcImg);

public:
	const cv::Mat& getMaskToKeepDrawLine() const;

private:
	/// Get binarization threshold with Otsu.
	double getThWithOtsu(
		const cv::Mat& bluredBhatImg,
		const cv::Rect& binROI
	);

};
