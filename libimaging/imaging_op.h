#pragma once

struct LumSample
{
	cv::Point m_point;
	uchar m_lum;

	LumSample() : m_lum(C_UCHAR(0)) { }
	LumSample(const int x, const int y, const uchar lum) : m_point(x, y), m_lum(lum) { }
};

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
std::vector<uchar> get_unmasked_data(
	const cv::Mat_<uchar>& image,
	const cv::Mat_<uchar>& mask,
	const cv::Rect& smpROI
);

/// マスクされていない画像の座標と輝度を取得する。
std::vector<LumSample> get_unmasked_point_and_lum(
	const cv::Mat_<uchar>& image,
	const cv::Mat_<uchar>& mask,
	const cv::Rect& smpROI
);

/// Clip as 8UC1 pixel value.
inline void clip_as_lum255(double& val)
{
	if (val < 0.0) {
		val = 0.0;
	}
	else if (val > 255.0) {
		val = 255.0;
	}
	else {
		/*pass*/
	}
}

/// 画像の最大輝度が255になるように画素の値をスカラー倍する。
cv::Mat_<uchar> stretch_to_white(const cv::Mat_<uchar>& image, double& minv, double& maxv);

/// ガンマ補正を行う。。
cv::Mat_<uchar> gamma_correction(const cv::Mat_<uchar>& image, double gamma);
