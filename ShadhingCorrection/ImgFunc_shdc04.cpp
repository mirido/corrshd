#include "stdafx.h"
#include "IImgFunc.h"
#include "ImgFuncBase.h"
#include "ImgFunc_whitening02.h"
#include "ImgFunc_uniform.h"
#include "ImgFunc_shdc04.h"

#include "../libnumeric/numericutil.h"
#include "../libimaging/geometryutil.h"
#include "../libimaging/shdcutil.h"

ImgFunc_shdc04::ImgFunc_shdc04(Param& param)
	: ImgFuncBase(param),
	m_imgFunc_Whiteing(param),
	m_imgFunc_uniform(param)
{
	m_imgFunc_Whiteing.needMaskToKeepDrawLine(true);
	m_imgFunc_Whiteing.doFinalInversion(false);
}

const char* ImgFunc_shdc04::getName() const
{
	return "shdc04";
}

const char* ImgFunc_shdc04::getSummary() const
{
	return "whitening02 (regression), then flatten blackness with polinomial regression.";
}

bool ImgFunc_shdc04::run(const cv::Mat& srcImg, cv::Mat& dstImg)
{
	// Whitening.
	cv::Mat invWhitenedImage;
	m_imgFunc_Whiteing.run(srcImg, invWhitenedImage);

	const cv::Mat& maskToKeepDrawLine = m_imgFunc_Whiteing.getMaskToKeepDrawLine();
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

	// Get background level with fr2.
	const cv::Rect binROI = get_bin_ROI(invWhitenedImage.size());
	cv::Mat fr2ROI = fr2(binROI);
	cv::Mat iwhROI = invWhitenedImage(binROI);
#if 0
	const cv::Scalar mean = cv::mean(iwhROI, fr2ROI);
	cout << "shdc04 mean=" << mean[0] << endl;
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
	m_imgFunc_uniform.run(invWhitenedImage, invUniformedImage);

	cv::bitwise_not(invUniformedImage, dstImg);

	return true;
}
