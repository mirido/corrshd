#include "stdafx.h"
#include "IImgFunc.h"
#include "ImgFuncBase.h"
#include "ImgFunc_whitening02.h"
#include "ImgFunc_uniform.h"
#include "ImgFunc_shdc04.h"

#include "../libnumeric/numericutil.h"
#include "../libimaging/geometryutil.h"
#include "../libimaging/shdcutil.h"

ImgFunc_shdc04::ImgFunc_shdc04(ParamPtr pParam)
	: ImgFunc_shdcWithUniform(pParam), m_whitening02(pParam)
{
	m_whitening02.setFlagToMakeMaskToKeepDrawLine(true);
	m_whitening02.setFinalInversionFlag(false);
}

const char* ImgFunc_shdc04::getName() const
{
	return "shdc04";
}

const char* ImgFunc_shdc04::getSummary() const
{
	return "whitening02 (regression), then flatten blackness with polinomial regression.";
}

bool ImgFunc_shdc04::whitening(
	const cv::Mat& srcImg,
	cv::Mat& dstImg,
	cv::Mat& maskToKeepDrawLine
)
{
	if (!m_whitening02.run(srcImg, dstImg)) {
		return false;
	}
	maskToKeepDrawLine = m_whitening02.getMaskToKeepDrawLine();
	return true;
}
