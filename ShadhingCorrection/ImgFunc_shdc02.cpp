#include "stdafx.h"
#include "IImgFunc.h"
#include "imaging_op.h"
#include "ImgFuncBase.h"
#include "ImgFunc_shdc02.h"

#include "../libnumeric/numericutil.h"
#include "shdcutil.h"

// [CONF] ROI size for determine binarization threshold (%)
#define BIN_ROI_RATIO		0.8

const char* ImgFunc_shdc02::getName() const
{
	return "shd02";
}

namespace
{
	cv::Rect get_bin_ROI(const cv::Mat& image)
	{
		const int width = image.cols;
		const int height = image.rows;
		const int newWidth = (int)std::round(width * BIN_ROI_RATIO);
		const int newHeight = (int)std::round(height * BIN_ROI_RATIO);

		return cv::Rect((width - newWidth) / 2, (height - newHeight) / 2, newWidth, newHeight);
	}

}	// namespace

bool ImgFunc_shdc02::run(const cv::Mat& srcImg, cv::Mat& dstImg)
{
#ifndef NDEBUG
	cout << std::setbase(10);
#endif

	const size_t nsamples = 100;

	cv::Mat maskForLineChg;
	if (!makeMaskImage(srcImg, maskForLineChg)) {
		return false;
	}
	//maskForLineChg = cv::Mat::zeros(srcImg.size(), CV_8UC1);		// Test.

	cv::Mat median3x3;
	cv::medianBlur(srcImg, median3x3, 3);
	dumpImg(median3x3, "median3x3", DBG_IMG_DIR);

	// Sample pixels on line.
	auto samplesOnLine = sampleImage(median3x3, maskForLineChg, nsamples);
	//for (auto it = samplesOnLine.begin(); it != samplesOnLine.end(); it++) { it->m_lum = C_UCHAR(64); }		// Test.

	// Sample pixels on background.
	cv::Mat maskForBgChg;
	cv::bitwise_not(maskForLineChg, maskForBgChg);
	auto samplesOnBg = sampleImage(median3x3, maskForBgChg, nsamples);
	//for (auto it = samplesOnBg.begin(); it != samplesOnBg.end(); it++) { it->m_lum = C_UCHAR(64 * 3); }		// Test.

	// Approximate lighting tilt on background.
	std::vector<double> cflistOnBg;
	if (!approximate_lighting_tilt_by_cubic_poly(samplesOnBg, cflistOnBg)) {
		return false;
	}

	// Approximate lighting tilt on line.
	std::vector<double> cflistOnLine;
	if (!approximate_lighting_tilt_by_cubic_poly(samplesOnLine, cflistOnLine)) {
		return false;
	}

#ifndef NDEBUG
	dumpAppxImg(srcImg.size(), cflistOnBg, "appx shade on bg", DBG_IMG_DIR);
	dumpAppxImg(srcImg.size(), cflistOnLine, "appx shade on line", DBG_IMG_DIR);
#endif

	dstImg = maskForBgChg;

	return true;
}

double ImgFunc_shdc02::getTh1FromBluredBlackHatResult(
	const cv::Mat& bluredBhatImg,
	const cv::Rect& binROI
)
{
	// Make ROI image for determine binarization threshold. (binROIImg)
	cv::Mat binROIImg = bluredBhatImg(binROI);

	// 平滑化結果binROIImgに対し、大津の方法で閾値th1を算出
	cv::Mat tmp;
	const double th1 = cv::threshold(binROIImg, tmp, 0, 255, cv::THRESH_OTSU);
	cout << "th1=" << th1 << endl;
	dumpImg(tmp, "ROI_img_for_det_th1", DBG_IMG_DIR);
	tmp.release();		// 2値化結果は使わない

#ifndef NDEBUG
	// get_unmasked_data()のテスト
	{
		cv::Mat nonmask = cv::Mat::ones(binROIImg.rows, binROIImg.cols, CV_8UC1);
		const std::vector<uchar> dbg_data = get_unmasked_data(binROIImg, nonmask);
		const int dbg_th1 = discriminant_analysis_by_otsu(dbg_data);
		cout << "dbg_th1=" << dbg_th1 << endl;
		if (dbg_th1 != (int)std::round(th1)) {
			throw std::logic_error("*** ERR ***");
		}
	}
#endif

	return th1;
}

bool ImgFunc_shdc02::makeMaskImage(const cv::Mat& srcImg, cv::Mat& mask)
{
	cv::Size dstSz = cv::Size(srcImg.cols, srcImg.rows);
	if (dstSz.empty()) {
		return false;
	}

	// 明るさのむらを均一化(ブラックハット演算、gray2)
	// クロージング結果 - 原画像、という演算なので、均一化とともに背景と線の輝度が反転する。
	// 以下の行末コメントは、dstSz.width == 800 の下でカーネルサイズを変えて実験した結果。
	//cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));		// 線がかすれる
	//cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
	//cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(10, 10));
	//cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(20, 20));		// 良好
	//cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(50, 50));		// 遅い
	const int knsz = (int)std::round(std::max(dstSz.width, dstSz.height) * 0.025);
	const cv::Size kernelSz = cv::Size(knsz, knsz);
	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, kernelSz);
	cv::Mat gray2;
	cv::morphologyEx(srcImg, gray2, cv::MORPH_BLACKHAT, kernel);
	dumpImg(gray2, "image_after_black_hat", DBG_IMG_DIR);

	// 以下、gray2を均一化画像と呼ぶ。

	// 均一化画像gray2平滑化(gray1)
	cv::Mat gray1;
	cv::blur(gray2, gray1, cv::Size(3, 3));

	// Get binarization threshold from ROI of gray1 image. (th1)
	const cv::Rect binROI = get_bin_ROI(gray1);
	const double th1 = getTh1FromBluredBlackHatResult(gray1, binROI);

	// マスク作成
	// 平滑化画像gray1の輝度th1以下を黒(0)、超過を白(255)にする(maskImg)
	cv::threshold(gray1, mask, th1, 255.0, cv::THRESH_BINARY);
	dumpImg(mask, "mask", DBG_IMG_DIR);

	return true;
}

/// Sample unmasked pixels.
std::vector<LumSample> ImgFunc_shdc02::sampleImage(
	const cv::Mat_<uchar>& image, const cv::Mat_<uchar>& mask, const size_t nsamples)
{
	auto data = get_unmasked_point_and_lum(image, mask);

	const size_t dataSz = data.size();
	if (dataSz <= nsamples) {
		return data;
	}

	std::vector<LumSample> samples(nsamples);
	for (size_t i = 0; i < nsamples; i++) {
		const size_t ii = (i * dataSz) / nsamples;
		samples[i] = data[ii];
	}

	return samples;
}

/// Method for debug. Dump approximation result visually.
void ImgFunc_shdc02::dumpAppxImg(
	const cv::Size imgSz,
	const std::vector<double>& cflist,
	const char* const caption,
	const char* const dstDir
)
{
	const int m = imgSz.height;
	const int n = imgSz.width;
	cv::Mat appxImg(m, n, CV_8UC1);
	for (size_t y = ZT(0); y < ZT(m); y++) {
		for (size_t x = ZT(0); x < ZT(n); x++) {
			double val = predict_by_qubic_poly(cflist, C_DBL(x), C_DBL(y));
			if (val < 0.0) {
				val = 0.0;
			}
			else if (val > 255.0) {
				val = 255.0;
			}
			appxImg.at<uchar>(C_INT(y), C_INT(x)) = (uchar)val;
		}
	}
	dumpImg(appxImg, caption, dstDir);
}
