#pragma once

class WithMaskNearZeroToZero
{
protected:
	cv::Mat m_maskNearZeroToZero;
	bool m_bNeedMaskNearZeroToZero;

public:
	WithMaskNearZeroToZero();

	void needMaskNearZeroToZero(const bool bNeed);

	const cv::Mat& getMaskNearZeroToZero() const;

protected:
	void makeMaskNearZeroToZero(
		const cv::Mat& srcImg,
		const cv::Mat& diffImg,
		const cv::InputArray mask = cv::noArray()
	);

};
