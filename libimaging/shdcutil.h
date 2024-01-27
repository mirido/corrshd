#pragma once

/// Sample pixels in image.
std::vector<LumSample> sample_pixels(
	const cv::Mat_<uchar>& image, const cv::Rect& smpROI, const int cyc_x, const int cyc_y);

/// Approximate lighting tilt by cubic polynomial.
bool approximate_lighting_tilt_by_cubic_poly(const std::vector<LumSample>& samples, std::vector<double>& cflist);

/// Predict by cubic polynomial.
double predict_by_qubic_poly(const std::vector<double>& cflist, const double x, const double y);

/// Stretch luminance.
void stretch_luminance(cv::Mat& image, const cv::Mat& maskForDLChg, std::vector<double>& cflistOnBg, const cv::Mat& blacknessMap);
