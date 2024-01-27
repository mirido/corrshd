#include "stdafx.h"
#include "IImgFunc.h"
#include "../libimaging/imaging_op.h"
#include "ImgFuncBase.h"
#include "ImgFunc_shdc02.h"

#include "../libnumeric/numericutil.h"
#include "../libimaging/geometryutil.h"
#include "../libimaging/shdcutil.h"

// [CONF] ROI size for determine binarization threshold (ratio)
#define BIN_ROI_RATIO		0.8

// [CONF] Kernel size for determine binarization threshold (ratio)
#define BIN_KERNEL_RATIO	0.025

const char* ImgFunc_shdc02::getName() const
{
	return "shd02";
}

const char* ImgFunc_shdc02::getSummary() const
{
	return "Corrects lighting tilt by cubic regression.";
}

namespace
{
	cv::Rect get_bin_ROI(const cv::Size& imgSz)
	{
		return get_scaled_rect_from_size(imgSz, BIN_ROI_RATIO);
	}

	cv::Size get_bin_kernel_size(const cv::Size& imgSz)
	{
		const int aveSz = std::max(imgSz.width, imgSz.height);
		const int knsz0 = (int)std::round(C_DBL(aveSz) * BIN_KERNEL_RATIO);
		const int knszOdd = 2 * ((knsz0 + 1) / 2) + 1;
		return cv::Size(knszOdd, knszOdd);
	}

	cv::Mat get_bin_kernel(const cv::Size& imgSz)
	{
		const cv::Size kernelSz = get_bin_kernel_size(imgSz);
		return cv::getStructuringElement(cv::MORPH_ELLIPSE, kernelSz);
	}

	size_t get_exp_nsamples(const cv::Rect& smpROI)
	{
		const cv::Size imgSz = cv::Size(smpROI.width, smpROI.height);
		const cv::Size kernelSz = get_bin_kernel_size(imgSz);
		const int hcnt = (imgSz.width + (kernelSz.width - 1)) / kernelSz.width;
		const int vcnt = (imgSz.height + (kernelSz.height - 1)) / kernelSz.height;
		return ZT(4) * ZT(hcnt) * ZT(vcnt);
	}

}	// namespace

bool ImgFunc_shdc02::run(const cv::Mat& srcImg, cv::Mat& dstImg)
{
	cv::Mat morphoTmpImg;

#ifndef NDEBUG
	cout << std::setbase(10);
#endif
	// Prepare source image for sampling.
	cv::Mat median3x3;
	cv::medianBlur(srcImg, median3x3, 5);
	dumpImg(median3x3, "median3x3", DBG_IMG_DIR);

	// Prepare kernel for dirate or erode.
	const cv::Mat kernel = get_bin_kernel(median3x3.size());

	// Sample pixels on background.
	cv::dilate(median3x3, morphoTmpImg, kernel);
	auto samplesOnBg = sampleImage(morphoTmpImg);
#ifndef NDEBUG
	cout << "samplesOnBg: size=" << samplesOnBg.size() << endl;
	plotSamples(morphoTmpImg, samplesOnBg, "samples on background", DBG_IMG_DIR);
#endif
	morphoTmpImg.release();

	// Approximate lighting tilt on background.
	std::vector<double> cflistOnBg;
	if (!approximate_lighting_tilt_by_cubic_poly(samplesOnBg, cflistOnBg)) {
		return false;
	}

	// Whitening.
	cv::Mat stdWhiteImg;
	predict_image(srcImg.size(), cflistOnBg, stdWhiteImg);
	cv::Mat invSrcImg = stdWhiteImg - srcImg;
	dumpImg(invSrcImg, "shading corrected image", DBG_IMG_DIR);
	stdWhiteImg.release();

	// Make mask for drawing line change.
	cv::Mat maskForDLChg;
	if (!makeMaskImage(srcImg, maskForDLChg)) {
		return false;
	}
	//maskForDLChg = cv::Mat::zeros(srcImg.size(), CV_8UC1);		// Test.

	// Sample pixels on drawing line.
	cv::dilate(invSrcImg, morphoTmpImg, kernel);
	auto samplesOnDL = sampleDrawLine(morphoTmpImg, maskForDLChg, samplesOnBg.size());
#ifndef NDEBUG
	cout << "samplesOnDL: size=" << samplesOnDL.size() << endl;
	plotSamples(morphoTmpImg, samplesOnDL, "samples on drawing line", DBG_IMG_DIR);
#endif
	morphoTmpImg.release();

	// Approximage blackness tilt on drawing line.
	std::vector<double> cflistOnDL;
	if (!approximate_lighting_tilt_by_cubic_poly(samplesOnDL, cflistOnDL)) {
		return false;
	}

	// Enhance drawing line.
	cv::Mat invBlacknessTiltImg;
	predict_image(invSrcImg.size(), cflistOnDL, invBlacknessTiltImg);
	dumpImg(invBlacknessTiltImg, "blackness tilt image", DBG_IMG_DIR);
	dstImg = invSrcImg;
	stretch_luminance(dstImg, maskForDLChg, invBlacknessTiltImg);

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
		const cv::Rect r(0, 0, binROIImg.cols, binROIImg.rows);
		const std::vector<uchar> dbg_data = get_unmasked_data(binROIImg, nonmask, r);
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
	const cv::Rect binROI = get_bin_ROI(gray1.size());
	const double th1 = getTh1FromBluredBlackHatResult(gray1, binROI);

	// マスク作成
	// 平滑化画像gray1の輝度th1以下を黒(0)、超過を白(255)にする(maskImg)
	cv::threshold(gray1, mask, th1, 255.0, cv::THRESH_BINARY);
	dumpImg(mask, "mask", DBG_IMG_DIR);

	return true;
}

/// Sample pixels.
std::vector<LumSample> ImgFunc_shdc02::sampleImage(const cv::Mat_<uchar>& image)
{
	const cv::Size kernelSz = get_bin_kernel_size(image.size());
	const cv::Rect smpROI = get_bin_ROI(image.size());
	return sample_pixels(image, smpROI, kernelSz.width, kernelSz.height);
}

/// Sample pixels on drawing line. 
std::vector<LumSample> ImgFunc_shdc02::sampleDrawLine(
	const cv::Mat_<uchar>& invImage, const cv::Mat_<uchar>& maskForDLChg, const size_t nsamples)
{
	const cv::Rect smpROI = get_bin_ROI(invImage.size());
	auto samplesOnDL = get_unmasked_point_and_lum(invImage, maskForDLChg, smpROI);

	const size_t sz = samplesOnDL.size();
	if (sz <= nsamples) {
		return samplesOnDL;
	}

	const size_t cyc = (sz + (nsamples - ZT(1))) / nsamples;
	const size_t expSz = (sz + (cyc - ZT(1))) / cyc;
	std::vector<LumSample> smpDL;
	smpDL.reserve(expSz);
	for (size_t i = 0; i < sz; i += cyc) {
		smpDL.push_back(samplesOnDL[i]);
	}
	assert(smpDL.size() == expSz);

	return smpDL;
}

//
//	For DEBUG
//

/// Dump approximation result visually. (For DEBUG.)
void ImgFunc_shdc02::dumpAppxImg(
	const cv::Mat srcImg,
	const std::vector<double>& cflist,
	const char* const caption,
	const char* const dstDir
)
{
	const int m = srcImg.rows;
	const int n = srcImg.cols;
	cv::Mat appxImg(m, n, CV_8UC1);
	for (size_t y = ZT(0); y < ZT(m); y++) {
		for (size_t x = ZT(0); x < ZT(n); x++) {
			double apxVal = predict_by_qubic_poly(cflist, C_DBL(x), C_DBL(y));
			if (apxVal < 0.0) {
				apxVal = 0.0;
			}
			else if (apxVal > 255.0) {
				apxVal = 255.0;
			}
			const uchar srcVal = srcImg.at<uchar>(C_INT(y), C_INT(x));
			//appxImg.at<uchar>(C_INT(y), C_INT(x)) = (uchar)(128.0 + 4 * (srcVal - apxVal));
			appxImg.at<uchar>(C_INT(y), C_INT(x)) = (uchar)(255.0 - 2 * std::abs(srcVal - apxVal));
		}
	}
	dumpImg(appxImg, caption, dstDir);
}

/// Plot sample points. (For DEBUG.)
void ImgFunc_shdc02::plotSamples(
	const cv::Mat_<uchar>& srcImg,
	const std::vector<LumSample>& samples,
	const char* const caption,
	const char* const dstDir
)
{
	const cv::Vec3b RED{ C_UCHAR(0), C_UCHAR(0), C_UCHAR(255) };

	cv::Mat kernel = get_bin_kernel(srcImg.size());
	cout << "srcImgSz=" << srcImg.size() << ", kernelSz=" << kernel.size() << endl;

	cv::Mat canvas;
	cv::cvtColor(srcImg, canvas, cv::COLOR_GRAY2BGR);
	//cv::rectangle(canvas, cv::Point(0, 0), cv::Point(canvas.cols - 1, canvas.rows - 1), RED, cv::FILLED);
	{
		cv::Mat thickerImgROI = canvas(cv::Rect(0, 0, kernel.cols, kernel.rows));
		cv::Mat kernelImg = (kernel.clone() * 10 + 128.0);
		cv::Mat kernelImgBGR;
		cv::cvtColor(kernelImg, kernelImgBGR, cv::COLOR_GRAY2BGR);
		kernelImgBGR.copyTo(thickerImgROI);
	}

	const cv::Vec3b markerColor = RED;
	for (auto it = samples.begin(); it != samples.end(); it++) {
		const int x = it->m_point.x;
		const int y = it->m_point.y;
		canvas.at<cv::Vec3b>(y, x) = markerColor;
	}

	dumpImg(canvas, caption, dstDir);
}
