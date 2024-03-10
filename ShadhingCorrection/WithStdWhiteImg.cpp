#include "stdafx.h"
#include "WithStdWhiteImg.h"

#include "../libimaging/imaging_op.h"
#include "../libnumeric/numericutil.h"
#include "bin_kernel.h"

WithStdWhiteImg::WithStdWhiteImg()
	: m_bMakeStdWhiteImg(false)
{
	/*pass*/
}

void WithStdWhiteImg::setFlagToMakeStdWhiteImg(const bool bMake)
{
	m_bMakeStdWhiteImg = bMake;
}

bool WithStdWhiteImg::getFlagToMakeStdWhiteImg() const
{
	return m_bMakeStdWhiteImg;
}

void WithStdWhiteImg::updateStdWhiteImg(const cv::Mat& newImg)
{
	if (m_bMakeStdWhiteImg) {
		m_stdWhiteImg = newImg.clone();
	}
	else {
		m_stdWhiteImg.release();
	}
}

void WithStdWhiteImg::releaseStdWhiteImg()
{
	m_stdWhiteImg.release();
}

const cv::Mat& WithStdWhiteImg::getStdWhiteImg() const
{
	assert(m_bMakeStdWhiteImg && !m_stdWhiteImg.empty());
	return m_stdWhiteImg;
}
