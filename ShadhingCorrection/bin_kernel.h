#pragma once

cv::Rect get_bin_ROI(const cv::Size& imgSz);

cv::Size get_bin_kernel_size(const cv::Size& imgSz);

cv::Mat get_bin_kernel(const cv::Size& imgSz);
