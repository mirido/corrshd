#include "stdafx.h"
#include "IImgFunc.h"
#include "ImgFuncBase.h"
#include "ImgFuncWithSampling.h"
#include "ImgFunc_whitening02Base.h"

#include "../libimaging/shdcutil.h"

ImgFunc_whitening02Base::ImgFunc_whitening02Base(Param& param)
	: ImgFuncWithSampling(param)
{
	/*pass*/
}

bool ImgFunc_whitening02Base::run(const cv::Mat& srcImg, cv::Mat& dstImg)
{
	// Prepare source image for sampling.
	cv::Mat median3x3;
	cv::medianBlur(srcImg, median3x3, 5);
	dumpImg(median3x3, "median3x3");

	// Prepare kernel for morphological operation.
	const cv::Mat kernel = get_bin_kernel(median3x3.size());

	// Sample pixels on background.
	cv::Mat morphoTmpImg;
	cv::dilate(median3x3, morphoTmpImg, kernel);
	auto samplesOnBg = sampleImage(morphoTmpImg);
#ifndef NDEBUG
	cout << "samplesOnBg: size=" << samplesOnBg.size() << endl;
	plotSamples(morphoTmpImg, samplesOnBg, "samples on background");
#endif

	// Approximate lighting tilt on background.
	std::vector<double> cflistOnBg;
	if (!approximate_lighting_tilt_by_cubic_poly(samplesOnBg, cflistOnBg)) {
		return false;
	}

	// Whitening.
	cv::Mat stdWhiteImg;
	predict_image(srcImg.size(), cflistOnBg, stdWhiteImg);
	if (m_bNeedStdWhiteImg) {
		updateStdWhiteImg(stdWhiteImg);
	}
	dumpImg(stdWhiteImg, "standard white image");
	// Following subtraction is achieved as saturation operation.
	dstImg = stdWhiteImg - srcImg;
	dumpImg(dstImg, "shading corrected image");

	morphoTmpImg.release();
	stdWhiteImg.release();

	if (m_bNeedMaskToKeepDrawLine) {
		makeMaskToKeepDrawLine(dstImg);
	}
	else {
		m_maskToKeepDrawLine.release();
	}

	if (m_bDoFinalInversion) {
		cv::bitwise_not(dstImg, dstImg);
	}

	return true;
}
