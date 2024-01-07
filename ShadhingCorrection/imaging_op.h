#pragma once

/// ‰æ‘œ‚ğ˜c‚ß‚éB
void warp_image(
	const cv::Mat& srcImage,
	cv::Mat& dstImage,
	const cv::Point2f srcROICorners[],
	const int npts,
	const cv::Size dstSz
);
