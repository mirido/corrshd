#include "stdafx.h"
#include "WithStdWhiteImg.h"

#include "../libimaging/imaging_op.h"
#include "../libnumeric/numericutil.h"
#include "bin_kernel.h"

WithStdWhiteImg::WithStdWhiteImg()
	: m_bNeedStdWhiteImg(false)
{
	/*pass*/
}

void WithStdWhiteImg::needStdWhiteImg(const bool bNeed)
{
	m_bNeedStdWhiteImg = bNeed;
}

void WithStdWhiteImg::updateStdWhiteImg(const cv::Mat& newImg)
{
	m_stdWhiteImg = newImg.clone();
}

const cv::Mat WithStdWhiteImg::getStdWhiteImg() const
{
	return m_stdWhiteImg;
}
