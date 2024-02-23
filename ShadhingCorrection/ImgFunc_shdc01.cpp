#include "stdafx.h"
#include "IImgFunc.h"
#include "ImgFuncBase.h"
#include "ImgFunc_shdc01.h"

#include "../libnumeric/numericutil.h"
#include "../libimaging/geometryutil.h"
#include "../libimaging/imaging_op.h"

ImgFunc_shdc01::ImgFunc_shdc01(Param& param)
	: ImgFuncBase(param), m_whitening01(param)
{
	m_whitening01.needMaskToKeepDrawLine(true);
	m_whitening01.doFinalInversion(false);
}

const char* ImgFunc_shdc01::getName() const
{
	return "shdc01";
}

const char* ImgFunc_shdc01::getSummary() const
{
	return "Shading correction with Black Top Hat, then gamma correction.";
}

bool ImgFunc_shdc01::run(const cv::Mat& srcImg, cv::Mat& dstImg)
{
	cout << std::setbase(10);

	// Whitening with Black Top Hat and get mask to keep draw line.
	cv::Mat gray2;
	if (!m_whitening01.run(srcImg, gray2)) {
		throw std::logic_error("*** ERR ***");
	}
	const cv::Mat maskToKeepDrawLine = m_whitening01.getMaskToKeepDrawLine();

	// �ȉ��Agray2���ψꉻ�摜�ƌĂԁB

	// �ψꉻ�摜gray2������(gray1)
	cv::Mat gray1;
	cv::blur(gray2, gray1, cv::Size(3, 3));
	
	// �ψꉻ�摜gray2 (�̕���������)�̃}�X�N����Ȃ���f�f�[�^�𒊏o(data)
	const cv::Rect smpROI = get_scaled_rect_from_size(gray1.size(), 1.0);
	std::vector<uchar> data = get_unmasked_data(gray1, maskToKeepDrawLine, smpROI, *(m_param.m_pMaskToAvoidFgObj));
	if (data.size() < 2) {
		return false;
	}

	// data�ɑ΂���Â̕��@�Ŕ��ʕ��͂����{
	const int th2 = discriminant_analysis_by_otsu(data);
	cout << "th2=" << th2 << endl;

	// data�̍ŏ��l���擾
	const int minv = *std::min_element(data.begin(), data.end());
	cout << "minv=" << minv << endl;

	const int th3 = minv + (int)((double)(th2 - minv) * 0.2);
	cout << "th3=" << th3 << endl;

	// �ψꉻ�摜gray2��臒lth3�ȉ�����(0)�A�������f�����摜�̂܂܂Ƃ���(gray2)
	cv::threshold(gray2, gray2, th3, 255.0, cv::THRESH_TOZERO);
	dumpImg(gray2, "threshold_by_th3");

	// gray2�̍ő�P�x��255�ɂȂ�悤�ɒ���(gray2)
	double img_minv, img_maxv;
	gray2 = stretch_to_white(gray2, img_minv, img_maxv);
	cout << "img_minv=" << img_minv << endl;
	cout << "img_maxv=" << img_maxv << endl;
	dumpImg(gray2, "stretch_to_white");

	// gray2���K���}�␳(gray2)
	gray2 = gamma_correction(gray2, 3.0);
	dumpImg(gray2, "gumma_correction");

	// gray2�𔒍����](dstImg)
	cv::bitwise_not(gray2, dstImg);

	return true;
}
