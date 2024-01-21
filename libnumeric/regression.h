#pragma once

/// Do polynomial regression.
void regress_poly_ex(
	const std::function<double(size_t)> xgetterFunc,
	const std::function<double(size_t)> ygetterFunc,
	const size_t nsamples,
	const int dim,
	cv::Mat& cflist
);

/// Do polynomial approximation. (cv::Point interface)
void regress_poly(const std::vector<cv::Point>& points, const int dim);
