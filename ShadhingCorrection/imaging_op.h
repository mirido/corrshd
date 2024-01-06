#pragma once

/// ‰æ‘œ‚ğ˜c‚ß‚éB
void warp_image(
	const cv::Mat& srcImage,
	const cv::Mat& dstImage,
	const cv::Point2f srcPts[],
	const int npts,
	const cv::Size dstSz
);
