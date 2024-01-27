#pragma once

/// Approximate lighting tilt by cubic polynomial.
bool approximate_lighting_tilt_by_cubic_poly(std::vector<LumSample>& samples, std::vector<double>& cflist);

/// Predict by cubic polynomial.
double predict_by_qubic_poly(const std::vector<double>& cflist, const double x, const double y);

/// Stretch luminance.
void stretch_luminance(cv::Mat& image, const cv::Mat& maskForDLChg, std::vector<double>& cflistOnBg/*, std::vector<double>& cflistOnDL*/);
