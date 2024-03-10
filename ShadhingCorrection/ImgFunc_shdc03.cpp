#include "stdafx.h"
#include "IImgFunc.h"
#include "ImgFuncBase.h"
#include "ImgFunc_whitening01.h"
#include "ImgFunc_uniform.h"
#include "ImgFunc_shdc03.h"

ImgFunc_shdc03::ImgFunc_shdc03(ParamPtr pParam)
	: ImgFunc_shdcWithUniform(pParam), m_whitening01(pParam)
{
	m_whitening01.setFlagToMakeMaskToKeepDrawLine(true);
	m_whitening01.setFinalInversionFlag(false);
}

const char* ImgFunc_shdc03::getName() const
{
	return "shdc03";
}

const char* ImgFunc_shdc03::getSummary() const
{
	return "whitening01 (Black Top Hat), then flatten blackness with polinomial regression.";
}

bool ImgFunc_shdc03::whitening(
	const cv::Mat& srcImg,
	cv::Mat& dstImg,
	cv::Mat& maskToKeepDrawLine
)
{
	if (!m_whitening01.run(srcImg, dstImg)) {
		return false;
	}
	maskToKeepDrawLine = m_whitening01.getMaskToKeepDrawLine();
	return true;
}
