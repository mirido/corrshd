#include "stdafx.h"
#include "IImgFunc.h"
#include "../libimaging/imaging_op.h"
#include "ImgFuncBase.h"
#include "ImgFunc_shdc02.h"

#include "../libnumeric/numericutil.h"
#include "../libimaging/geometryutil.h"
#include "../libimaging/shdcutil.h"

ImgFunc_shdc02::ImgFunc_shdc02(ParamPtr pParam)
	: ImgFuncWithSampling(pParam), m_whitening02(pParam), m_whitening01(pParam)
{
	m_whitening02.setFlagToMakeMaskToKeepDrawLine(false);
	m_whitening02.setFinalInversionFlag(false);
	m_whitening01.setFlagToMakeMaskToKeepDrawLine(true);
	m_whitening01.setFinalInversionFlag(false);
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

	// Settings to keep the same behavior as before.
	const cv::Size dstSz = srcImg.size();
	const int knsz = (int)std::round(std::max(dstSz.width, dstSz.height) * 0.025);
	const cv::Size kernelSz = cv::Size(knsz, knsz);
	cv::Mat kernel2 = cv::getStructuringElement(cv::MORPH_ELLIPSE, kernelSz);
	m_whitening01.setCustomKernel(kernel2);

	// Make mask for drawing line change.
	cv::Mat gray2;
	if (!m_whitening01.run(srcImg, gray2)) {
		return false;
	}
	gray2.release();
	const cv::Mat maskForDLChg = m_whitening01.getMaskToKeepDrawLine();

	// Prepare kernel for dirate or erode.
	const cv::Mat kernel = get_bin_kernel(invSrcImg.size());

	// Get the size of samplesOnBg m_whitening01 used
	// to keep the same behavior as before.
	const size_t nsamples = m_whitening02.getLastSizeOfSamplesOnBG();
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
	const cv::Rect smpROI = get_bin_ROI(invImage.size(), m_pParam->m_ratioOfSmpROIToImgSz);
	auto samplesOnDL = get_unmasked_point_and_lum(invImage, maskForDLChg, smpROI, m_pParam->m_maskToAvoidFgObj);

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
