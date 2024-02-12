#include "stdafx.h"
#include "WithMaskToKeepDrawLine.h"

#include "../libimaging/imaging_op.h"
#include "../libnumeric/numericutil.h"
#include "bin_kernel.h"

WithMaskToKeepDrawLine::WithMaskToKeepDrawLine()
	: m_thToKeepDrawLine(0.0), m_bNeedMaskToKeepDrawLine(false)
{
	/*pass*/
}

void WithMaskToKeepDrawLine::needMaskToKeepDrawLine(const bool bNeed)
{
	m_bNeedMaskToKeepDrawLine = bNeed;
}

void WithMaskToKeepDrawLine::makeMaskToKeepDrawLine(const cv::Mat& srcImg)
{
	// This method assumes that background pixels of srcImg are almost equalized to 0.

	// �ψꉻ�摜gray2������(bluredImg)
	cv::Mat bluredImg;
	cv::blur(srcImg, bluredImg, cv::Size(3, 3));

	// Get binarization threshold from ROI of bluredImg. (th1)
	const cv::Rect binROI = get_bin_ROI(bluredImg.size());
	m_thToKeepDrawLine = getThWithOtsu(bluredImg, binROI);

	// �}�X�N�쐬
	// �������摜bluredImg�̋P�xth1�ȉ�����(0)�A���߂�(255)�ɂ���(maskImg)
	cv::threshold(bluredImg, m_maskToKeepDrawLine, m_thToKeepDrawLine, 255.0, cv::THRESH_BINARY);
}

const cv::Mat& WithMaskToKeepDrawLine::getMaskToKeepDrawLine() const
{
	return m_maskToKeepDrawLine;
}

/// Get binarization threshold with Otsu.
double WithMaskToKeepDrawLine::getThWithOtsu(
	const cv::Mat& bluredBhatImg,
	const cv::Rect& binROI
)
{
	// Make ROI image for determine binarization threshold. (binROIImg)
	cv::Mat binROIImg = bluredBhatImg(binROI);

	// ����������binROIImg�ɑ΂��A��Â̕��@��臒lth1���Z�o
	cv::Mat tmp;
	const double th1 = cv::threshold(binROIImg, tmp, 0, 255, cv::THRESH_OTSU);
	//cout << "th1=" << th1 << endl;
	//dumpImg(tmp, "ROI_img_for_det_th1", DBG_IMG_DIR);
	tmp.release();		// 2�l�����ʂ͎g��Ȃ�

#ifndef NDEBUG
	// get_unmasked_data()�̃e�X�g
	{
		cv::Mat nonmask = cv::Mat::ones(binROIImg.rows, binROIImg.cols, CV_8UC1);
		const cv::Rect r(0, 0, binROIImg.cols, binROIImg.rows);
		const std::vector<uchar> dbg_data = get_unmasked_data(binROIImg, nonmask, r);
		const int dbg_th1 = discriminant_analysis_by_otsu(dbg_data);
		cout << "dbg_th1=" << dbg_th1 << endl;
		if (dbg_th1 != (int)std::round(th1)) {
			throw std::logic_error("*** ERR ***");
		}
	}
#endif

	return th1;
}
