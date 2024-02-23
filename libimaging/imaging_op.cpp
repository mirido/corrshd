#include "stdafx.h"
#include "imaging_op.h"

#include "../libnumeric/numericutil.h"
#include "geometryutil.h"

/// �O���[�X�P�[���摜�ɕϊ�����B
bool conv_color_to_gray(const cv::Mat& srcImage, cv::Mat& grayImage)
{
	switch (srcImage.channels()) {
	case 1:
		// (�\�[�X�摜��gray scale�摜)
		grayImage = srcImage;
		break;
	case 3:
		// (�\�[�X�摜��BGR�摜)
		cv::cvtColor(srcImage, grayImage, cv::COLOR_BGR2GRAY);
		break;
	default:
		return false;
	}

	return true;
}

/// BGR�摜�ɕϊ�����B
bool conv_color_to_BGR(const cv::Mat& srcImage, cv::Mat& BGRImage)
{
	switch (srcImage.channels()) {
	case 1:
		// (�\�[�X�摜��gray scale�摜)
		cv::cvtColor(srcImage, BGRImage, cv::COLOR_GRAY2BGR);
		break;
	case 3:
		// (�\�[�X�摜��BGR�摜)
		BGRImage = srcImage;
		break;
	default:
		return false;
	}

	return true;
}

/// �摜��c�߂�B
void warp_image(
	const cv::Mat& srcImage,
	cv::Mat& dstImage,
	const cv::Point2f srcROICorners[],
	const int npts,
	const cv::Size dstSz
)
{
	if (!(npts == 4)) {
		throw std::logic_error("*** ERR ***");
	}

	// �ϊ����4�_
	const float left = (float)dstSz.width;
	const float btm = (float)dstSz.height;
	cv::Point2f dstPts[4];
	dstPts[0] = cv::Point2f(0.0F, 0.0F);
	dstPts[1] = cv::Point2f(left, 0.0F);
	dstPts[2] = cv::Point2f(left, btm);
	dstPts[3] = cv::Point2f(0.0f, btm);

	// �ϊ��s��擾
	const cv::Mat M = cv::getPerspectiveTransform(srcROICorners, dstPts);

	// �ϊ����{
	cv::warpPerspective(srcImage, dstImage, M, dstSz);
}

/// �}�X�N����Ă��Ȃ��摜�̉�f�f�[�^���擾����B
std::vector<uchar> get_unmasked_data(
	const cv::Mat_<uchar>& image,
	const cv::Mat_<uchar>& mask,
	const cv::Rect& smpROI,
	const cv::InputArray globalMask
)
{
	const int width = image.cols;
	const int height = image.rows;
	if (!(mask.cols == width && mask.rows == height)) {
		throw std::logic_error("*** ERR ***");
	}

	const cv::Rect ROI = clip_rect_into_image(smpROI, width, height);
	int sx, sy, ex, ey;
	decompose_rect(ROI, sx, sy, ex, ey);

	std::vector<uchar> data;
	if (globalMask.empty()) {
		for (int y = sy; y < ey; y++) {
			for (int x = sx; x < ex; x++) {
				if (mask(y, x) > C_UCHAR(0)) {
					data.push_back(image(y, x));
				}
			}
		}
	}
	else {
		const cv::Mat gmask = globalMask.getMat();
		if (!(gmask.size() == image.size())) {
			throw std::logic_error("*** ERR ***");
		}
		for (int y = sy; y < ey; y++) {
			for (int x = sx; x < ex; x++) {
				if (mask(y, x) > C_UCHAR(0) && gmask.at<uchar>(y, x) > C_UCHAR(0)) {
					data.push_back(image(y, x));
				}
			}
		}
	}

	return data;
}

/// �}�X�N����Ă��Ȃ��摜�̍��W�ƋP�x���擾����B
std::vector<LumSample> get_unmasked_point_and_lum(
	const cv::Mat_<uchar>& image,
	const cv::Mat_<uchar>& mask,
	const cv::Rect& smpROI,
	const cv::InputArray globalMask
)
{
	const int width = image.cols;
	const int height = image.rows;
	if (!(mask.cols == width && mask.rows == height)) {
		throw std::logic_error("*** ERR ***");
	}

	const cv::Rect ROI = clip_rect_into_image(smpROI, width, height);
	int sx, sy, ex, ey;
	decompose_rect(ROI, sx, sy, ex, ey);

	std::vector<LumSample> data;
	if (globalMask.empty()) {
		for (int y = sy; y < ey; y++) {
			for (int x = sx; x < ex; x++) {
				if (mask(y, x) > C_UCHAR(0)) {
					LumSample val;
					val.m_point = cv::Point(x, y);
					val.m_lum = image(y, x);
					data.push_back(val);
				}
			}
		}
	}
	else {
		const cv::Mat gmask = globalMask.getMat();
		if (!(gmask.size() == image.size())) {
			throw std::logic_error("*** ERR ***");
		}
		for (int y = sy; y < ey; y++) {
			for (int x = sx; x < ex; x++) {
				if (mask(y, x) > C_UCHAR(0) && gmask.at<uchar>(y, x) > C_UCHAR(0)) {
					LumSample val;
					val.m_point = cv::Point(x, y);
					val.m_lum = image(y, x);
					data.push_back(val);
				}
			}
		}
	}

	return data;
}

/// �摜�̍ő�P�x��255�ɂȂ�悤�ɉ�f�̒l���X�J���[�{����B
cv::Mat_<uchar> stretch_to_white(const cv::Mat_<uchar>& image, double& minv, double& maxv)
{
	cv::minMaxLoc(image, &minv, &maxv);
	minv = (int)minv;
	maxv = (int)maxv;

	return image * (255.0 / maxv);
}

/// �K���}�␳���s���B�B
cv::Mat_<uchar> gamma_correction(const cv::Mat_<uchar>& image, double gamma)
{
	cv::Mat gammaLUT = cv::Mat(1, 256, CV_8U);

	const double inv_gamma = 1.0 / gamma;
	for (int i = 0; i < 256; i++) {
		gammaLUT.at<uchar>(0, i) = (uchar)(255 * std::pow(((double)i / 255.0), inv_gamma));
	}
#ifndef NDEBUG
	cout << "Gamma LUT" << endl;
	for (int i = 0; i < 256; i++) {
		cout << i << "," << (int)gammaLUT.at<uchar>(0, i) << endl;
	}
#endif

	// �Ǎ��摜���K���}�ϊ�
	cv::Mat result;
	cv::LUT(image, gammaLUT, result);

	return result;
}
