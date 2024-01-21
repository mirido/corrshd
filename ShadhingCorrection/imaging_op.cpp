#include "stdafx.h"
#include "imaging_op.h"

#include "../libnumeric/numericutil.h"

/// グレースケール画像に変換する。
bool conv_color_to_gray(const cv::Mat& srcImage, cv::Mat& grayImage)
{
	switch (srcImage.channels()) {
	case 1:
		// (ソース画像がgray scale画像)
		grayImage = srcImage;
		break;
	case 3:
		// (ソース画像がBGR画像)
		cv::cvtColor(srcImage, grayImage, cv::COLOR_BGR2GRAY);
		break;
	default:
		return false;
	}

	return true;
}

/// BGR画像に変換する。
bool conv_color_to_BGR(const cv::Mat& srcImage, cv::Mat& BGRImage)
{
	switch (srcImage.channels()) {
	case 1:
		// (ソース画像がgray scale画像)
		cv::cvtColor(srcImage, BGRImage, cv::COLOR_GRAY2BGR);
		break;
	case 3:
		// (ソース画像がBGR画像)
		BGRImage = srcImage;
		break;
	default:
		return false;
	}

	return true;
}

/// 画像を歪める。
void warp_image(
	const cv::Mat& srcImage,
	cv::Mat& dstImage,
	const cv::Point2f srcROICorners[],
	const int npts,
	const cv::Size dstSz
)
{
	if (!(npts == 4)) {
		throw std::logic_error("*** ERR ***");
	}

	// 変換後の4点
	const float left = (float)dstSz.width;
	const float btm = (float)dstSz.height;
	cv::Point2f dstPts[4];
	dstPts[0] = cv::Point2f(0.0F, 0.0F);
	dstPts[1] = cv::Point2f(left, 0.0F);
	dstPts[2] = cv::Point2f(left, btm);
	dstPts[3] = cv::Point2f(0.0f, btm);

	// 変換行列取得
	const cv::Mat M = cv::getPerspectiveTransform(srcROICorners, dstPts);

	// 変換実施
	cv::warpPerspective(srcImage, dstImage, M, dstSz);
}

/// マスクされていない画像の画素データを取得する。
std::vector<uchar> get_unmasked_data(const cv::Mat_<uchar>& image, const cv::Mat_<uchar>& mask)
{
	const int width = image.cols;
	const int height = image.rows;
	if (!(mask.cols == width && mask.rows == height)) {
		throw std::logic_error("*** ERR ***");
	}

	std::vector<uchar> data;
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			if (mask(y, x) > 0) {
				data.push_back(image(y, x));
			}
		}
	}

	return data;
}

/// 画像の最大輝度が255になるように画素の値をスカラー倍する。
cv::Mat_<uchar> stretch_to_white(const cv::Mat_<uchar>& image, double& minv, double& maxv)
{
	cv::minMaxLoc(image, &minv, &maxv);
	minv = (int)minv;
	maxv = (int)maxv;

	return image * (255.0 / maxv);
}

/// ガンマ補正を行う。。
cv::Mat_<uchar> gamma_correction(const cv::Mat_<uchar>& image, double gamma)
{
	cv::Mat gammaLUT = cv::Mat(1, 256, CV_8U);

	const double inv_gamma = 1.0 / gamma;
	for (int i = 0; i < 256; i++) {
		gammaLUT.at<uchar>(0, i) = (uchar)(255 * std::pow(((double)i / 255.0), inv_gamma));
	}
#ifndef NDEBUG
	cout << "Gamma LUT" << endl;
	for (int i = 0; i < 256; i++) {
		cout << i << "," << (int)gammaLUT.at<uchar>(0, i) << endl;
	}
#endif

	// 読込画像をガンマ変換
	cv::Mat result;
	cv::LUT(image, gammaLUT, result);

	return result;
}
