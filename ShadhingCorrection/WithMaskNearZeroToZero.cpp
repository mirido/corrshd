#include "stdafx.h"
#include "WithMaskNearZeroToZero.h"

#include "bin_kernel.h"

WithMaskNearZeroToZero::WithMaskNearZeroToZero()
	: m_bNeedMaskNearZeroToZero(false)
{
	/*pass*/
}

void WithMaskNearZeroToZero::needMaskNearZeroToZero(const bool bNeed)
{
	m_bNeedMaskNearZeroToZero = bNeed;
}

void WithMaskNearZeroToZero::makeMaskNearZeroToZero(
	const cv::Mat& srcImg,
	const cv::Mat& diffImg,
	const cv::InputArray& maskToLimitRange
)
{
	if (!(srcImg.size() == diffImg.size())) {
		throw std::logic_error("*** ERR ***");
	}
	const cv::Rect binROI = get_bin_ROI(diffImg.size());
	const cv::Mat limitedDiffImg = diffImg(binROI);

	cv::Mat mean, stddev;
	cv::meanStdDev(limitedDiffImg, mean, stddev, maskToLimitRange);
	const double fmean = mean.at<double>(0, 0);
	const double fstddev = stddev.at<double>(0, 0);
	cout << "  mean=" << fmean << endl;
	cout << "stddev=" << fstddev << endl;

	cv::threshold(srcImg, m_maskNearZeroToZero, fmean + fstddev, 255.0, cv::THRESH_BINARY);
}

const cv::Mat& WithMaskNearZeroToZero::getMaskNearZeroToZero() const
{
	return m_maskNearZeroToZero;
}
