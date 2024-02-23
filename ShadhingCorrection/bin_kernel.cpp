#include "stdafx.h"
#include "bin_kernel.h"

#include "../libimaging/geometryutil.h"

// [CONF] Kernel size for determine binarization threshold (ratio)
#define BIN_KERNEL_RATIO	0.025

cv::Rect get_bin_ROI(const cv::Size& imgSz, const double ratioOfSmpROIToImgSz)
{
	return get_scaled_rect_from_size(imgSz, ratioOfSmpROIToImgSz);
}

cv::Size get_bin_kernel_size(const cv::Size& imgSz)
{
	const int ageImgSz = (imgSz.width, imgSz.height) / 2;
	const int knsz0 = (int)std::round(C_DBL(ageImgSz) * BIN_KERNEL_RATIO);
	const int knszOdd = std::max(3, 2 * ((knsz0 + 1) / 2) + 1);
	return cv::Size(knszOdd, knszOdd);
}

cv::Mat get_bin_kernel(const cv::Size& imgSz)
{
	const cv::Size kernelSz = get_bin_kernel_size(imgSz);
	return cv::getStructuringElement(cv::MORPH_ELLIPSE, kernelSz);
}
