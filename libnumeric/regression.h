#pragma once

/// Do polynomial regression.
void regress_poly_core(
	const cv::Mat& G,
	const cv::Mat& y,
	cv::Mat& cflist
);

/// Do polynomial regression. (Output as std::vector<double>.)
void regress_poly_core(
	const cv::Mat& G,
	const cv::Mat& y,
	std::vector<double>& cflist
);

/// Do polynomial regression. (cv::Point interface)
void regress_poly(const std::vector<cv::Point2d>& points, const int degree, std::vector<double>& cflist);
