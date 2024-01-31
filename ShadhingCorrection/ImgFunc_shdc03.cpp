#include "stdafx.h"
#include "IImgFunc.h"
#include "ImgFuncBase.h"
#include "ImgFunc_whitening02.h"
#include "ImgFunc_uniform.h"
#include "ImgFunc_shdc03.h"

#include "../libnumeric/numericutil.h"
#include "../libimaging/geometryutil.h"
#include "../libimaging/shdcutil.h"

ImgFunc_shdc03::ImgFunc_shdc03()
{
	m_imgFunc_Whiteing.needMaskNearZeroToZero(true);
	m_imgFunc_Whiteing.doFinalInversion(false);
}

const char* ImgFunc_shdc03::getName() const
{
	return "shd03";
}

const char* ImgFunc_shdc03::getSummary() const
{
	return "Corrects lighting tilt by cubic regression.";
}

bool ImgFunc_shdc03::run(const cv::Mat& srcImg, cv::Mat& dstImg)
{
	// Whitening.
	cv::Mat invWhitenedImage;
	m_imgFunc_Whiteing.run(srcImg, invWhitenedImage);

	const cv::Mat& maskNearZeroToZero = m_imgFunc_Whiteing.getMaskNearZeroToZero();
	dumpImg(maskNearZeroToZero, "mask near 0 to 0", DBG_IMG_DIR);

	const cv::Mat kernel = get_bin_kernel(srcImg.size());

	// Eliminate bakcground noize.
#if 1
	const cv::Rect binROI = get_bin_ROI(invWhitenedImage.size());
	cv::Mat preMask;
	m_imgFunc_uniform.makeMaskImage(invWhitenedImage, preMask);
	cv::Mat maskToCoverDL1 = cv::Mat::zeros(preMask.size(), CV_8UC1);
	preMask(binROI).copyTo(maskToCoverDL1(binROI));

	cv::Mat maskToCoverDL2;
	cv::dilate(maskToCoverDL1, maskToCoverDL2, kernel);
	maskToCoverDL1.release();
	cv::Mat maskToCoverDL3;
	cv::dilate(maskToCoverDL2, maskToCoverDL3, kernel);

	cv::threshold(maskToCoverDL2, maskToCoverDL2, 0.0, 255.0, cv::THRESH_BINARY_INV);
	cv::threshold(maskToCoverDL3, maskToCoverDL3, 0.0, 255.0, cv::THRESH_BINARY);
	cv::bitwise_and(maskToCoverDL3, maskToCoverDL2, maskToCoverDL3);
	maskToCoverDL2.release();

	cv::bitwise_and(maskToCoverDL3, maskNearZeroToZero, maskToCoverDL3);
	dumpImg(maskToCoverDL3, "maskToCoverDL3", DBG_IMG_DIR);

	const cv::Scalar mean =	cv::mean(invWhitenedImage, maskToCoverDL3);
	cout << "shdc03 mean=" << mean[0] << endl;

	invWhitenedImage -= mean[0];

	cv::dilate(preMask, maskToCoverDL2, kernel, cv::Point(-1, -1), 1);
	cv::threshold(maskToCoverDL2, maskToCoverDL2, 0.0, 255, cv::THRESH_BINARY);
	cv::bitwise_and(invWhitenedImage, maskToCoverDL2, invWhitenedImage);
#elif 1
	cv::Mat maskToCleanupBG;
	const double th2 = cv::threshold(invWhitenedImage, maskToCleanupBG, 0, 255, cv::THRESH_OTSU);
	cout << "th2=" << th2 << endl;
	cv::dilate(maskToCleanupBG, maskToCleanupBG, kernel);
	dumpImg(maskToCleanupBG, "mask for cleanup background (thicked)", DBG_IMG_DIR);
	cv::bitwise_and(maskToCleanupBG, maskNearZeroToZero, maskToCleanupBG);
	//cv::bitwise_or(maskToCleanupBG, maskNearZeroToZero, maskToCleanupBG);
	dumpImg(maskToCleanupBG, "mask for cleanup background (slimed)", DBG_IMG_DIR);
	cv::bitwise_and(invWhitenedImage, maskToCleanupBG, invWhitenedImage);
#elif 0
	cv::Mat maskForCleanupBG;
	cv::dilate(invWhitenedImage, maskForCleanupBG, kernel);
	const double th2 = cv::threshold(invWhitenedImage, maskForCleanupBG, 0, 255, cv::THRESH_OTSU);
	cout << "th2=" << th2 << endl;
	cv::dilate(maskForCleanupBG, maskForCleanupBG, kernel);
	dumpImg(maskForCleanupBG, "mask for cleanup background", DBG_IMG_DIR);
	cv::bitwise_and(invWhitenedImage, maskForCleanupBG, invWhitenedImage);
#elif 0
	cv::Mat tmp;
	const double th2 = cv::threshold(invWhitenedImage, tmp, 0, 255, cv::THRESH_OTSU);
	cout << "th2=" << th2 << endl;
	tmp.release();
	cv::threshold(invWhitenedImage, invWhitenedImage, th2 * 0.5, 255.0, cv::THRESH_TOZERO);
#endif
	dumpImg(invWhitenedImage, "cleanup image", DBG_IMG_DIR);

	// Uniform.
	cv::Mat invUniformedImage;
	m_imgFunc_uniform.run(invWhitenedImage, invUniformedImage);

	cv::bitwise_not(invUniformedImage, dstImg);

	return true;
}
