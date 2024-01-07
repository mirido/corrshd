#pragma once

/// グレースケール画像に変換する。
bool conv_color_to_gray(const cv::Mat& srcImage, cv::Mat& grayImage);

/// BGR画像に変換する。
bool conv_color_to_BGR(const cv::Mat& srcImage, cv::Mat& BGRImage);

/// 画像を歪める。
void warp_image(
	const cv::Mat& srcImage,
	cv::Mat& dstImage,
	const cv::Point2f srcROICorners[],
	const int npts,
	const cv::Size dstSz
);

/// マスクされていない画像の画素データを取得する。
std::vector<uchar> get_unmasked_data(const cv::Mat_<uchar>& image, const cv::Mat_<uchar>& mask);
