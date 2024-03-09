#include "stdafx.h"
#include "IImgFunc.h"
#include "ImgFuncBase.h"
#include "ImgFunc_whitening01.h"

#include "../libnumeric/numericutil.h"
#include "../libimaging/geometryutil.h"
#include "../libimaging/imaging_op.h"

ImgFunc_whitening01::ImgFunc_whitening01(ParamPtr pParam)
	: ImgFuncBase(pParam)
{
	/*pass*/
}

const char* ImgFunc_whitening01::getName() const
{
	return "whitening01";
}

const char* ImgFunc_whitening01::getSummary() const
{
	return "Shading correction with Black Top Hat algorithm.";
}

bool ImgFunc_whitening01::run(const cv::Mat& srcImg, cv::Mat& dstImg)
{
	// 明るさのむらを均一化(ブラックハット演算、gray2)
	// クロージング結果 - 原画像、という演算なので、均一化とともに背景と線の輝度が反転する。
	// 以下の行末コメントは、srcImg.size().width == 800 の下でカーネルサイズを変えて実験した結果。
	//cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));		// 線がかすれる
	//cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
	//cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(10, 10));
	//cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(20, 20));		// 良好
	//cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(50, 50));		// 遅い
	const cv::Mat kernel = (!m_customKernel.empty()) ? m_customKernel : get_bin_kernel(srcImg.size());
	cv::morphologyEx(srcImg, dstImg, cv::MORPH_BLACKHAT, kernel);
	dumpImg(dstImg, "image_after_black_hat");

	if (m_bNeedMaskToKeepDrawLine) {
		makeMaskToKeepDrawLine(dstImg, m_pParam->m_ratioOfSmpROIToImgSz, m_pParam->m_maskToAvoidFgObj);
	}
	else {
		m_maskToKeepDrawLine.release();
	}

	if (m_bDoFinalInversion) {
		cv::bitwise_not(dstImg, dstImg);
	}

	return true;
}

/// Method to keep the same behavior as before refactoring on shdc02.
void ImgFunc_whitening01::setCustomKernel(const cv::Mat& kernel)
{
	m_customKernel = kernel;
}
