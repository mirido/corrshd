#include "stdafx.h"
#include "IImgFunc.h"
#include "ImgFuncBase.h"
#include "ImgFunc_uniform.h"

#include "../libnumeric/numericutil.h"
#include "../libimaging/shdcutil.h"

const char* ImgFunc_uniform::getName() const
{
	return "uniforming";
}

const char* ImgFunc_uniform::getSummary() const
{
	return "Uniform peak level of pixels.";
}

bool ImgFunc_uniform::run(const cv::Mat& srcImg, cv::Mat& dstImg)
{
	// This method assumes that background pixels of srcImg are almost equalized to 0.

	// Make mask for drawing line change.
	cv::Mat maskForDLChg;
	makeMaskImage(srcImg, maskForDLChg);

	// Prepare source image for sampling.
	//cv::Mat median3x3;
	//cv::medianBlur(srcImg, median3x3, 5);
	//dumpImg(median3x3, "median3x3", DBG_IMG_DIR);

	// Prepare kernel for dirate or erode.
	const cv::Mat kernel = get_bin_kernel(srcImg.size());

	// Sample pixels on drawing line.
	cv::Mat morphoTmpImg;
	cv::dilate(srcImg, morphoTmpImg, kernel);
	auto samplesOnDL = sampleDrawLine(morphoTmpImg, maskForDLChg);
#ifndef NDEBUG
	cout << "samplesOnDL: size=" << samplesOnDL.size() << endl;
	plotSamples(morphoTmpImg, samplesOnDL, "samples on drawing line", DBG_IMG_DIR);
#endif
	morphoTmpImg.release();

	maskForDLChg.release();

	// Approximage blackness tilt on drawing line.
	std::vector<double> cflistOnDL;
	if (!approximate_lighting_tilt_by_cubic_poly(samplesOnDL, cflistOnDL)) {
		return false;
	}

	// Enhance drawing line.
	cv::Mat invBlacknessTiltImg;
	predict_image(srcImg.size(), cflistOnDL, invBlacknessTiltImg);
	dumpImg(invBlacknessTiltImg, "inv blackness tilt image", DBG_IMG_DIR);
	dstImg = srcImg;
	stretch_luminance(dstImg, invBlacknessTiltImg);

	return true;
}

/// Get binarization threshold with Otsu.
double ImgFunc_uniform::getTh1WithOtsu(
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

/// Make mask image for drawing line change.
void ImgFunc_uniform::makeMaskImage(const cv::Mat& srcImg, cv::Mat& mask)
{
	// This method assumes that background pixels of srcImg are almost equalized to 0.

	// 均一化画像gray2平滑化(bluredImg)
	cv::Mat bluredImg;
	cv::blur(srcImg, bluredImg, cv::Size(3, 3));

	// Get binarization threshold from ROI of bluredImg. (th1)
	const cv::Rect binROI = get_bin_ROI(bluredImg.size());
	const double th1 = getTh1WithOtsu(bluredImg, binROI);

	// マスク作成
	// 平滑化画像bluredImgの輝度th1以下を黒(0)、超過を白(255)にする(maskImg)
	cv::threshold(bluredImg, mask, th1, 255.0, cv::THRESH_BINARY);
	dumpImg(mask, "mask", DBG_IMG_DIR);
}

/// Sample pixels on drawing line. 
std::vector<LumSample> ImgFunc_uniform::sampleDrawLine(
	const cv::Mat_<uchar>& image,
	const cv::Mat_<uchar>& maskForDLChg
)
{
	const cv::Size kernelSz = get_bin_kernel_size(image.size());
	const cv::Rect smpROI = get_bin_ROI(image.size());

	const int cntx = get_num_grid_points(smpROI.width, kernelSz.width);
	const int cnty = get_num_grid_points(smpROI.height, kernelSz.height);
	const size_t nsamples = ZT(cntx) * ZT(cnty);

	auto samplesOnDL = get_unmasked_point_and_lum(image, maskForDLChg, smpROI);

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
