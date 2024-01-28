#include "stdafx.h"
#include "IImgFunc.h"
#include "ImgFuncBase.h"
#include "ImgFunc_shdc02.h"

#include "../libnumeric/numericutil.h"
#include "imaging_op.h"

// [CONF] ROI size for determine binarization threshold (%)
#define BIN_ROI_RATIO		0.8

const char* ImgFunc_shdc02::getName() const
{
	return "shd02";
}

namespace
{
	cv::Rect get_bin_ROI(const cv::Mat& image)
	{
		const int width = image.cols;
		const int height = image.rows;
		const int newWidth = (int)std::round(width * BIN_ROI_RATIO);
		const int newHeight = (int)std::round(height * BIN_ROI_RATIO);

		return cv::Rect((width - newWidth) / 2, (height - newHeight) / 2, newWidth, newHeight);
	}

}	// namespace

bool ImgFunc_shdc02::run(const cv::Mat& srcImg, cv::Mat& dstImg)
{
	cv::Mat tmp;

	cv::Mat gray1 = srcImg;
	cv::Size dstSz = cv::Size(srcImg.cols, srcImg.rows);

#ifndef NDEBUG
	cout << std::setbase(10);
#endif

	// ���邳�̂ނ���ψꉻ(�u���b�N�n�b�g���Z�Agray2)
	// �N���[�W���O���� - ���摜�A�Ƃ������Z�Ȃ̂ŁA�ψꉻ�ƂƂ��ɔw�i�Ɛ��̋P�x�����]����B
	// �ȉ��̍s���R�����g�́AdstSz.width == 800 �̉��ŃJ�[�l���T�C�Y��ς��Ď����������ʁB
	//cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));		// �����������
	//cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
	//cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(10, 10));
	//cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(20, 20));		// �ǍD
	//cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(50, 50));		// �x��
	const int knsz = (int)std::round(std::max(dstSz.width, dstSz.height) * 0.025);
	const cv::Size kernelSz = cv::Size(knsz, knsz);
	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, kernelSz);
	cv::Mat gray2;
	cv::morphologyEx(gray1, gray2, cv::MORPH_BLACKHAT, kernel);
	dumpImg(gray2, "image_after_black_hat", DBG_IMG_DIR);

	// �ȉ��Agray2���ψꉻ�摜�ƌĂԁB

	// �ψꉻ�摜gray2������(gray1)
	cv::blur(gray2, gray1, cv::Size(3, 3));

	// Get binarization threshold from ROI of gray1 image. (th1)
	const cv::Rect binROI = get_bin_ROI(gray1);
	const double th1 = getTh1FromBluredBlackHatResult(gray1, binROI);

	// �}�X�N�쐬
	// �������摜gray1�̋P�xth1�ȉ�����(0)�A���߂�(255)�ɂ���(mask)
	cv::Mat mask;
	cv::threshold(gray1, mask, th1, 255.0, cv::THRESH_BINARY);
	dumpImg(mask, "mask", DBG_IMG_DIR);

	// �ψꉻ�摜��gray1�̃}�X�N����Ȃ���f�f�[�^�𒊏o(data)
	std::vector<uchar> data = get_unmasked_data(gray1, mask);
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
	dumpImg(gray2, "threshold_by_th3", DBG_IMG_DIR);

	// gray2�̍ő�P�x��255�ɂȂ�悤�ɒ���(gray2)
	double img_minv, img_maxv;
	gray2 = stretch_to_white(gray2, img_minv, img_maxv);
	cout << "img_minv=" << img_minv << endl;
	cout << "img_maxv=" << img_maxv << endl;
	dumpImg(gray2, "stretch_to_white", DBG_IMG_DIR);

	// gray2���K���}�␳(gray2)
	gray2 = gamma_correction(gray2, 3.0);
	dumpImg(gray2, "gumma_correction", DBG_IMG_DIR);

	// gray2�𔒍����](dstImg)
	cv::bitwise_not(gray2, dstImg);

	return true;
}

double ImgFunc_shdc02::getTh1FromBluredBlackHatResult(
	const cv::Mat& bluredBhatImg,
	const cv::Rect& binROI
)
{
	// Make ROI image for determine binarization threshold. (binROIImg)
	cv::Mat binROIImg = bluredBhatImg(binROI);

	// ����������binROIImg�ɑ΂��A��Â̕��@��臒lth1���Z�o
	cv::Mat tmp;
	const double th1 = cv::threshold(binROIImg, tmp, 0, 255, cv::THRESH_OTSU);
	cout << "th1=" << th1 << endl;
	dumpImg(tmp, "ROI_img_for_det_th1", DBG_IMG_DIR);
	tmp.release();		// 2�l�����ʂ͎g��Ȃ�

#ifndef NDEBUG
	// get_unmasked_data()�̃e�X�g
	{
		cv::Mat nonmask = cv::Mat::ones(binROIImg.rows, binROIImg.cols, CV_8UC1);
		const std::vector<uchar> dbg_data = get_unmasked_data(binROIImg, nonmask);
		const int dbg_th1 = discriminant_analysis_by_otsu(dbg_data);
		cout << "dbg_th1=" << dbg_th1 << endl;
		if (dbg_th1 != (int)std::round(th1)) {
			throw std::logic_error("*** ERR ***");
		}
	}
#endif

	return th1;
}
