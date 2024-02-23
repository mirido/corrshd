#include "stdafx.h"
#include "IImgFunc.h"
#include "ImgFuncBase.h"
#include "ImgFunc_uniform.h"

#include "../libnumeric/numericutil.h"
#include "../libimaging/shdcutil.h"

ImgFunc_uniform::ImgFunc_uniform(Param& param)
	: ImgFuncWithSampling(param)
{
	/*pass*/
}

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

	// Make mask to keep draw line.
	makeMaskToKeepDrawLine(srcImg, *(m_param.m_pRatioOfSmpROIToImgSz), *(m_param.m_pMaskToAvoidFgObj));
	cv::Mat maskToKeepDL = m_maskToKeepDrawLine;

	// Prepare kernel for dirate or erode.
	const cv::Mat kernel = get_bin_kernel(srcImg.size());

	// Sample pixels on drawing line.
	cv::Mat morphoTmpImg;
	cv::dilate(srcImg, morphoTmpImg, kernel);
	auto samplesOnDL = sampleDrawLine(morphoTmpImg, maskToKeepDL);
#ifndef NDEBUG
	cout << "samplesOnDL: size=" << samplesOnDL.size() << endl;
	plotSamples(morphoTmpImg, samplesOnDL, "samples on drawing line");
#endif
	morphoTmpImg.release();

	maskToKeepDL.release();

	// Approximage blackness tilt on drawing line.
	std::vector<double> cflistOnDL;
	if (!approximate_lighting_tilt_by_cubic_poly(samplesOnDL, cflistOnDL)) {
		return false;
	}

	// Enhance drawing line.
	cv::Mat invBlacknessTiltImg;
	predict_image(srcImg.size(), cflistOnDL, invBlacknessTiltImg);
	dumpImg(invBlacknessTiltImg, "inv blackness tilt image");
	dstImg = srcImg;
	stretch_luminance(dstImg, invBlacknessTiltImg);

	return true;
}

/// Sample pixels on drawing line. 
std::vector<LumSample> ImgFunc_uniform::sampleDrawLine(
	const cv::Mat_<uchar>& image,
	const cv::Mat_<uchar>& maskToKeepDL
)
{
	const cv::Size kernelSz = get_bin_kernel_size(image.size());
	const cv::Rect smpROI = get_bin_ROI(image.size(), *(m_param.m_pRatioOfSmpROIToImgSz));

	const int cntx = get_num_grid_points(smpROI.width, kernelSz.width);
	const int cnty = get_num_grid_points(smpROI.height, kernelSz.height);
	const size_t nsamples = ZT(cntx) * ZT(cnty);

	auto samplesOnDL = get_unmasked_point_and_lum(image, maskToKeepDL, smpROI, *(m_param.m_pMaskToAvoidFgObj));

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
