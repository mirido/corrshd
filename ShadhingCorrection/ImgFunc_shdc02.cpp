#include "stdafx.h"
#include "IImgFunc.h"
#include "ImgFuncBase.h"
#include "ImgFunc_shdc02.h"

#include "../libnumeric/numericutil.h"
#include "imaging_op.h"

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
	cv::Mat tmp;

	cv::Mat gray1 = srcImg;
	cv::Size dstSz = cv::Size(srcImg.cols, srcImg.rows);

#ifndef NDEBUG
	cout << std::setbase(10);
#endif

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
	cv::morphologyEx(gray1, gray2, cv::MORPH_BLACKHAT, kernel);
	dumpImg(gray2, "image_after_black_hat", DBG_IMG_DIR);

	// 以下、gray2を均一化画像と呼ぶ。

	// 均一化画像gray2平滑化(gray1)
	cv::blur(gray2, gray1, cv::Size(3, 3));

	// Get binarization threshold from ROI of gray1 image. (th1)
	const cv::Rect binROI = get_bin_ROI(gray1);
	const double th1 = getTh1FromBluredBlackHatResult(gray1, binROI);

	// マスク作成
	// 平滑化画像gray1の輝度th1以下を黒(0)、超過を白(255)にする(mask)
	cv::Mat mask;
	cv::threshold(gray1, mask, th1, 255.0, cv::THRESH_BINARY);
	dumpImg(mask, "mask", DBG_IMG_DIR);

	// 均一化画像のgray1のマスクされない画素データを抽出(data)
	std::vector<uchar> data = get_unmasked_data(gray1, mask);
	if (data.size() < 2) {
		return false;
	}

	// dataに対し大津の方法で判別分析を実施
	const int th2 = discriminant_analysis_by_otsu(data);
	cout << "th2=" << th2 << endl;

	// dataの最小値を取得
	const int minv = *std::min_element(data.begin(), data.end());
	cout << "minv=" << minv << endl;

	const int th3 = minv + (int)((double)(th2 - minv) * 0.2);
	cout << "th3=" << th3 << endl;

	// 均一化画像gray2の閾値th3以下を黒(0)、超える画素を原画像のままとする(gray2)
	cv::threshold(gray2, gray2, th3, 255.0, cv::THRESH_TOZERO);
	dumpImg(gray2, "threshold_by_th3", DBG_IMG_DIR);

	// gray2の最大輝度が255になるように調整(gray2)
	double img_minv, img_maxv;
	gray2 = stretch_to_white(gray2, img_minv, img_maxv);
	cout << "img_minv=" << img_minv << endl;
	cout << "img_maxv=" << img_maxv << endl;
	dumpImg(gray2, "stretch_to_white", DBG_IMG_DIR);

	// gray2をガンマ補正(gray2)
	gray2 = gamma_correction(gray2, 3.0);
	dumpImg(gray2, "gumma_correction", DBG_IMG_DIR);

	// gray2を白黒反転(dstImg)
	cv::bitwise_not(gray2, dstImg);

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
