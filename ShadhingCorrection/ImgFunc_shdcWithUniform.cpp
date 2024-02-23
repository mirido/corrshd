#include "stdafx.h"
#include "IImgFunc.h"
#include "ImgFuncBase.h"
#include "ImgFunc_uniform.h"
#include "ImgFunc_shdcWithUniform.h"

#include "../libnumeric/numericutil.h"
#include "../libimaging/geometryutil.h"
#include "../libimaging/shdcutil.h"

ImgFunc_shdcWithUniform::ImgFunc_shdcWithUniform(Param& param)
	: ImgFuncBase(param),
	m_uniform(param)
{
	/*pass*/
}

bool ImgFunc_shdcWithUniform::run(const cv::Mat& srcImg, cv::Mat& dstImg)
{
	// Whitening.
	cv::Mat invWhitenedImage, maskToKeepDrawLine;
	if (!whitening(srcImg, invWhitenedImage, maskToKeepDrawLine)) {
		return false;
	}
	dumpImg(maskToKeepDrawLine, "mask to keep draw line");

	const cv::Mat kernel = get_bin_kernel(srcImg.size());

	// Make dilated image dl1 and d2.
	cv::Mat dl1;
	cv::dilate(maskToKeepDrawLine, dl1, kernel);
	dumpImg(dl1, "Dilated image dl1");
	cv::Mat dl2;
	cv::dilate(dl1, dl2, kernel);
	dumpImg(dl2, "Dilated image dl2");

	// Make fringe mask fr2.
	// We can assume that fr2 does not contain any drawing lines.
	cv::Mat fr2 = dl2 - dl1;
	dumpImg(fr2, "Fringe mask fr2");

	// Exclude disturbance pixel ranges from fringe mask.
	if (!m_param.m_pMaskToAvoidFgObj->empty()) {
		if (!(m_param.m_pMaskToAvoidFgObj->size() == fr2.size())) {
			throw std::logic_error("*** ERR ***");
		}
		cv::bitwise_and(fr2, *(m_param.m_pMaskToAvoidFgObj), fr2);
	}

	// Get background level with fr2.
	const cv::Rect binROI = get_bin_ROI(invWhitenedImage.size(), *(m_param.m_pRatioOfSmpROIToImgSz));
	cv::Mat fr2ROI = fr2(binROI);
	cv::Mat iwhROI = invWhitenedImage(binROI);
#if 0
	const cv::Scalar mean = cv::mean(iwhROI, fr2ROI);
	cout << "shdc03 mean=" << mean[0] << endl;
	const double bglevel = mean[0];
#else
	const cv::Scalar mean, stddev;
	cv::meanStdDev(iwhROI, mean, stddev, fr2ROI);
	cout << "  mean=" << mean[0] << endl;
	cout << "stddev=" << stddev[0] << endl;
	const double bglevel = mean[0] + stddev[0];
#endif

	// Cancel background level.
	invWhitenedImage -= bglevel;

	// Clean up background.
	cv::bitwise_and(invWhitenedImage, dl2, invWhitenedImage);
	dumpImg(invWhitenedImage, "cleanup image");

	// Uniform.
	cv::Mat invUniformedImage;
	if (!m_uniform.run(invWhitenedImage, invUniformedImage)) {
		return false;
	}

	cv::bitwise_not(invUniformedImage, dstImg);

	return true;
}
