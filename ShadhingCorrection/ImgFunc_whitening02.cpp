#include "stdafx.h"
#include "IImgFunc.h"
#include "ImgFuncBase.h"
#include "ImgFuncWithSampling.h"
#include "ImgFunc_whitening02.h"

#include "../libimaging/shdcutil.h"

ImgFunc_whitening02::ImgFunc_whitening02(Param& param)
	: ImgFuncWithSampling(param)
{
	/*pass*/
}

const char* ImgFunc_whitening02::getName() const
{
	return "whitening02";
}

const char* ImgFunc_whitening02::getSummary() const
{
	return "Shading correction with cubic polynomial regression.";
}

bool ImgFunc_whitening02::run(const cv::Mat& srcImg, cv::Mat& dstImg)
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

/// Sample pixels.
std::vector<LumSample> ImgFunc_whitening02::sampleImage(const cv::Mat_<uchar>& image)
{
	const cv::Size kernelSz = get_bin_kernel_size(image.size());
	const cv::Rect smpROI = get_bin_ROI(image.size());
	return sample_pixels(image, smpROI, kernelSz.width, kernelSz.height);
}
