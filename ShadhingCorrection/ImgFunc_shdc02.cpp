#include "stdafx.h"
#include "IImgFunc.h"
#include "../libimaging/imaging_op.h"
#include "ImgFuncBase.h"
#include "ImgFunc_shdc02.h"

#include "../libnumeric/numericutil.h"
#include "../libimaging/geometryutil.h"
#include "../libimaging/shdcutil.h"

// [CONF] ROI size for determine binarization threshold (ratio)
#define BIN_ROI_RATIO		0.8

// [CONF] Kernel size for determine binarization threshold (ratio)
#define BIN_KERNEL_RATIO	0.025

const char* ImgFunc_shdc02::getName() const
{
	return "shd02";
}

namespace
{
	cv::Rect get_bin_ROI(const cv::Mat& image)
	{
		return get_scaled_rect_from_size(image.size(), BIN_ROI_RATIO);
	}

	cv::Size get_bin_kernel_size(const cv::Size& imgSz)
	{
		const int aveSz = std::max(imgSz.width, imgSz.height);
		const int knsz0 = (int)std::round(C_DBL(aveSz) * 0.025);
		const int knszOdd = 2 * ((knsz0 + 1) / 2) + 1;
		return cv::Size(knszOdd, knszOdd);
	}

	cv::Mat get_bin_kernel(const cv::Mat& image)
	{
		const cv::Size imgSz = image.size();
		const cv::Size kernelSz = get_bin_kernel_size(imgSz);
		return cv::getStructuringElement(cv::MORPH_ELLIPSE, kernelSz);
	}

	size_t get_exp_nsamples(const cv::Rect& smpROI)
	{
		const cv::Size imgSz = cv::Size(smpROI.width, smpROI.height);
		const cv::Size kernelSz = get_bin_kernel_size(imgSz);
		const int hcnt = (imgSz.width + (kernelSz.width - 1)) / kernelSz.width;
		const int vcnt = (imgSz.height + (kernelSz.height - 1)) / kernelSz.height;
		return ZT(4) * ZT(hcnt) * ZT(vcnt);
	}

}	// namespace

bool ImgFunc_shdc02::run(const cv::Mat& srcImg, cv::Mat& dstImg)
{
	cv::Mat tmpImg;

#ifndef NDEBUG
	cout << std::setbase(10);
#endif

	// Make mask for drawing line change.
	cv::Mat maskForDLChg;
	if (!makeMaskImage(srcImg, maskForDLChg)) {
		return false;
	}
	//maskForDLChg = cv::Mat::zeros(srcImg.size(), CV_8UC1);		// Test.

	// Prepare source image for sampling.
	cv::Mat median3x3;
	cv::medianBlur(srcImg, median3x3, 3);
	dumpImg(median3x3, "median3x3", DBG_IMG_DIR);

	// Prepare kernel for dirate or erode.
	const cv::Mat kernel = get_bin_kernel(median3x3);

#if 0
	// Sample pixels on drawing line.
	cv::erode(median3x3, tmpImg, kernel);
	auto samplesOnDL = sampleImage(tmpImg, maskForDLChg);
#ifndef NDEBUG
	cout << "samplesOnDL: size=" << samplesOnDL.size() << endl;
	plotSamples(tmpImg, samplesOnDL, "samples on drawing line", DBG_IMG_DIR);
#endif
	tmpImg.release();
#endif

	// Sample pixels on background.
	cv::dilate(median3x3, tmpImg, kernel);
	cv::Mat maskForBgChg;
	cv::bitwise_not(maskForDLChg, maskForBgChg);
	auto samplesOnBg = sampleImage(tmpImg, maskForBgChg);
#ifndef NDEBUG
	cout << "samplesOnBg: size=" << samplesOnBg.size() << endl;
	plotSamples(tmpImg, samplesOnBg, "samples on background", DBG_IMG_DIR);
#endif
	tmpImg.release();

	// Approximate lighting tilt on background.
	std::vector<double> cflistOnBg;
	if (!approximate_lighting_tilt_by_cubic_poly(samplesOnBg, cflistOnBg)) {
		return false;
	}

#if 0
	// Approximate lighting tilt on drawing line.
	std::vector<double> cflistOnDL;
	if (!approximate_lighting_tilt_by_cubic_poly(samplesOnDL, cflistOnDL)) {
		return false;
	}
#endif

	dstImg = srcImg.clone();
	stretch_luminance(dstImg, maskForDLChg, cflistOnBg/*, cflistOnDL*/);

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

bool ImgFunc_shdc02::makeMaskImage(const cv::Mat& srcImg, cv::Mat& mask)
{
	cv::Size dstSz = cv::Size(srcImg.cols, srcImg.rows);
	if (dstSz.empty()) {
		return false;
	}

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
	cv::morphologyEx(srcImg, gray2, cv::MORPH_BLACKHAT, kernel);
	dumpImg(gray2, "image_after_black_hat", DBG_IMG_DIR);

	// �ȉ��Agray2���ψꉻ�摜�ƌĂԁB

	// �ψꉻ�摜gray2������(gray1)
	cv::Mat gray1;
	cv::blur(gray2, gray1, cv::Size(3, 3));

	// Get binarization threshold from ROI of gray1 image. (th1)
	const cv::Rect binROI = get_bin_ROI(gray1);
	const double th1 = getTh1FromBluredBlackHatResult(gray1, binROI);

	// �}�X�N�쐬
	// �������摜gray1�̋P�xth1�ȉ�����(0)�A���߂�(255)�ɂ���(maskImg)
	cv::threshold(gray1, mask, th1, 255.0, cv::THRESH_BINARY);
	dumpImg(mask, "mask", DBG_IMG_DIR);

	return true;
}

/// Sample unmasked pixels.
std::vector<LumSample> ImgFunc_shdc02::sampleImage(
	const cv::Mat_<uchar>& image, const cv::Mat_<uchar>& mask)
{
	const cv::Rect smpROI = get_bin_ROI(image);
#if 0
	auto data = get_unmasked_point_and_lum(image, mask, smpROI);

	const size_t dataSz = data.size();
	const int vtImgSz = (int)std::round(std::sqrt(dataSz));
	const size_t nsamples = get_exp_nsamples(cv::Rect(0, 0, vtImgSz, vtImgSz));
	if (dataSz <= nsamples) {
		return data;
	}

	std::vector<LumSample> samples(nsamples);
	for (size_t i = 0; i < nsamples; i++) {
		const size_t ii = (i * dataSz) / nsamples;
		samples[i] = data[ii];
	}
#else
	(void)(mask);
	std::vector<LumSample> samples;
	const cv::Size kernelSz = get_bin_kernel_size(image.size());
	const int knw = kernelSz.width;
	const int knh = kernelSz.height;
	samples.reserve((ZT(smpROI.width) / ZT(knw)) * ((ZT(smpROI.height) / ZT(knh))));
	int sx, sy, ex, ey;
	decompose_rect(smpROI, sx, sy, ex, ey);
	for (int y = sy; y < ey; y += knh) {
		for (int x = sx; x < ex; x += knw) {
			const uchar lum = image.at<uchar>(y, x);
			samples.push_back(LumSample(x, y, lum));
		}
	}
#endif

	return samples;
}

//
//	For DEBUG
//

/// Dump approximation result visually. (For DEBUG.)
void ImgFunc_shdc02::dumpAppxImg(
	const cv::Mat srcImg,
	const std::vector<double>& cflist,
	const char* const caption,
	const char* const dstDir
)
{
	const int m = srcImg.rows;
	const int n = srcImg.cols;
	cv::Mat appxImg(m, n, CV_8UC1);
	for (size_t y = ZT(0); y < ZT(m); y++) {
		for (size_t x = ZT(0); x < ZT(n); x++) {
			double apxVal = predict_by_qubic_poly(cflist, C_DBL(x), C_DBL(y));
			if (apxVal < 0.0) {
				apxVal = 0.0;
			}
			else if (apxVal > 255.0) {
				apxVal = 255.0;
			}
			const uchar srcVal = srcImg.at<uchar>(C_INT(y), C_INT(x));
			//appxImg.at<uchar>(C_INT(y), C_INT(x)) = (uchar)(128.0 + 4 * (srcVal - apxVal));
			appxImg.at<uchar>(C_INT(y), C_INT(x)) = (uchar)(255.0 - 2 * std::abs(srcVal - apxVal));
		}
	}
	dumpImg(appxImg, caption, dstDir);
}

/// Plot sample points. (For DEBUG.)
void ImgFunc_shdc02::plotSamples(
	const cv::Mat_<uchar>& srcImg,
	const std::vector<LumSample>& samples,
	const char* const caption,
	const char* const dstDir
)
{
	const cv::Vec3b RED{ C_UCHAR(0), C_UCHAR(0), C_UCHAR(255) };

	cv::Mat kernel = get_bin_kernel(srcImg);
	cout << "srcImgSz=" << srcImg.size() << ", kernelSz=" << kernel.size() << endl;

	cv::Mat canvas;
	cv::cvtColor(srcImg, canvas, cv::COLOR_GRAY2BGR);
	//cv::rectangle(canvas, cv::Point(0, 0), cv::Point(canvas.cols - 1, canvas.rows - 1), RED, cv::FILLED);
	{
		cv::Mat thickerImgROI = canvas(cv::Rect(0, 0, kernel.cols, kernel.rows));
		cv::Mat kernelImg = (kernel.clone() * 10 + 128.0);
		cv::Mat kernelImgBGR;
		cv::cvtColor(kernelImg, kernelImgBGR, cv::COLOR_GRAY2BGR);
		kernelImgBGR.copyTo(thickerImgROI);
	}

	const cv::Vec3b markerColor = RED;
	for (auto it = samples.begin(); it != samples.end(); it++) {
		const int x = it->m_point.x;
		const int y = it->m_point.y;
		canvas.at<cv::Vec3b>(y, x) = markerColor;
	}

	dumpImg(canvas, caption, dstDir);
}
