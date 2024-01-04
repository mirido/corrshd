#include "stdafx.h"
#include "imaging_op.h"

/// ‰æ‘œ‚ğ˜c‚ß‚éB
void warp_image(
	const cv::Mat& srcImage,
	const cv::Mat& dstImage,
	const cv::Point2f srcPts[],
	const int npts,
	const double relWidth,
	const double relHeight,
	const int outputWidth
)
{
	if (!(npts == 4)) {
		throw std::logic_error("*** ERR ***");
	}

	// •ÏŠ·Œã‚Ì4“_
	const int nOutWidth = outputWidth;
	const int nOutHeight = (int)std::round(((double)outputWidth * relHeight) / relWidth);
	const float left = (float)nOutWidth;
	const float btm = (float)nOutHeight;
	cv::Point2f dstPts[4];
	dstPts[0] = cv::Point2f(0.0F, 0.0F);
	dstPts[1] = cv::Point2f(left, 0.0F);
	dstPts[2] = cv::Point2f(left, btm);
	dstPts[3] = cv::Point2f(0.0f, btm);

	// •ÏŠ·s—ñæ“¾
	const cv::Mat M = cv::getPerspectiveTransform(srcPts, dstPts);

	// •ÏŠ·À{
	cv::warpPerspective(srcImage, dstImage, M, cv::Size(nOutWidth, nOutHeight));
}
