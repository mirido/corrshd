#pragma once

/// Sample pixels in cv::Vec3b image.
std::vector<cv::Vec3b> sample_Vec3b_pixels(
	const cv::Mat_<cv::Vec3b>& Vec3bImg, const cv::Rect& smpROI, const int cyc_x, const int cyc_y);

int get_Vec3b_distance(const cv::Vec3b& a, const cv::Vec3b& b);

cv::Vec3b blend_Vec3b(const cv::Vec3b& a, const cv::Vec3b& b);

void split_near_or_far(
	const std::vector<cv::Vec3b>& samples,
	const cv::Vec3b& center,
	const int sr,
	std::vector<size_t>& lsNear,
	std::vector<size_t>& lsFar
);

size_t estimate_mean_with_sample_ex(
	const std::vector<cv::Vec3b>& samples, cv::Vec3b& curMean, const int sr);

cv::Vec3b estimate_mean_with_sample(
	const std::vector<cv::Vec3b>& samples, const int sr);

void make_mask_from_sample(
	const cv::Mat& Vec3bImg, const std::vector<cv::Vec3b>& samples, const int sr, cv::Mat& mask);
