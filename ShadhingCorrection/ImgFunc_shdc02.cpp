#include "stdafx.h"
#include "IImgFunc.h"
#include "../libimaging/imaging_op.h"
#include "ImgFuncBase.h"
#include "ImgFunc_shdc02.h"

#include "../libnumeric/numericutil.h"
#include "../libimaging/geometryutil.h"
#include "../libimaging/shdcutil.h"

ImgFunc_shdc02::ImgFunc_shdc02(Param& param)
	: ImgFuncWithSampling(param), m_whitening02(param), m_whitening01(param)
{
	m_whitening02.needMaskToKeepDrawLine(false);
	m_whitening02.doFinalInversion(false);
	m_whitening01.needMaskToKeepDrawLine(true);
	m_whitening01.doFinalInversion(false);
}

const char* ImgFunc_shdc02::getName() const
{
	return "shdc02";
}

const char* ImgFunc_shdc02::getSummary() const
{
	return "Shading correction with cubic polynomial regression, then flatten blackness and masking.";
}

bool ImgFunc_shdc02::run(const cv::Mat& srcImg, cv::Mat& dstImg)
{
	cout << std::setbase(10);

	// Whitening with cubic polynomial regression.
	cv::Mat invSrcImg;
	if (!m_whitening02.run(srcImg, invSrcImg)) {
		return false;
	}

	// Make mask for drawing line change.
	cv::Mat gray2;
	if (!m_whitening01.run(srcImg, gray2)) {
		return false;
	}
	gray2.release();
	const cv::Mat maskForDLChg = m_whitening01.getMaskToKeepDrawLine();

	// Prepare kernel for dirate or erode.
	const cv::Mat kernel = get_bin_kernel(invSrcImg.size());

	// Estimate the size of samplesOnBg m_whitening01 used.
	// It is no longer possible to directly obtain the value
	// because the module m_whitening02 was separated by refactoring.
	// If the values are similar, there is no problem, so estimate.
	const int cyc = std::min(kernel.cols, kernel.rows);
	const size_t nsamples = ZT(cv::countNonZero(maskForDLChg) / cyc);
	cout << "nsamples = " << nsamples << " (estimation of samplesOnBg.size())" << endl;

	// Sample pixels on drawing line.
	cv::Mat morphoTmpImg;
	cv::dilate(invSrcImg, morphoTmpImg, kernel);
	auto samplesOnDL = sampleDrawLine(morphoTmpImg, maskForDLChg, nsamples);
	cout << "samplesOnDL: size=" << samplesOnDL.size() << endl;
	plotSamples(morphoTmpImg, samplesOnDL, "samples on drawing line");
	morphoTmpImg.release();

	// Approximage blackness tilt on drawing line.
	std::vector<double> cflistOnDL;
	if (!approximate_lighting_tilt_by_cubic_poly(samplesOnDL, cflistOnDL)) {
		return false;
	}

	// Enhance drawing line.
	cv::Mat invBlacknessTiltImg;
	predict_image(invSrcImg.size(), cflistOnDL, invBlacknessTiltImg);
	dumpImg(invBlacknessTiltImg, "blackness tilt image");
	dstImg = invSrcImg;
	stretch_and_invert_luminance(dstImg, maskForDLChg, invBlacknessTiltImg);

	return true;
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
