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
	const cv::InputArray mask_in
)
{
	if (!(srcImg.size() == diffImg.size())) {
		throw std::logic_error("*** ERR ***");
	}
	if (!(diffImg.type() == CV_16SC1 || diffImg.type() == CV_32SC1)) {
		throw std::logic_error("*** ERR ***");
	}

	const cv::Mat mask = (mask_in.empty()) ? cv::Mat::ones(srcImg.size(), CV_8UC1) : mask_in.getMat();

	const cv::Rect binROI = get_bin_ROI(diffImg.size());
	const cv::Mat diffInROI = diffImg(binROI);
	const cv::Mat maskInROI = mask(binROI);

	const int npixels = cv::countNonZero(mask);
	cout << "npixels=" << npixels << endl;

	cv::Scalar mean, stddev;
	cv::meanStdDev(diffInROI, mean, stddev, maskInROI);
	const double fmean = mean[0];
	const double fstddev = stddev[0];
	const double fstderr = fstddev / sqrt(std::max(1, npixels - 1));
	cout << "  mean=" << fmean << endl;
	cout << "stddev=" << fstddev << endl;
	cout << "stderr=" << fstderr << endl;

	const double th = std::min(0.0, fmean) + fstddev;
	cv::threshold(srcImg, m_maskNearZeroToZero, th, 255.0, cv::THRESH_BINARY);
}

const cv::Mat& WithMaskNearZeroToZero::getMaskNearZeroToZero() const
{
	return m_maskNearZeroToZero;
}
