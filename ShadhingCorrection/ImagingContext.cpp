#include "stdafx.h"
#include "ImagingCanvas.h"
#include "ClickedPointList.h"
#include "ImagingContext.h"

#include "numericutil.h"
#include "imaging_op.h"
#include "geometryutil.h"

// [DBGSW] ���ԉ摜�ۑ���
#define DBG_IMG_DIR		"C:\\usr2\\debug\\"

// [CONF] �N���b�N�ʒu�̋�����臒l
// �����|�C���g�Ƃ̃}���n�b�^���������ȉ��̒l�ȉ��Ȃ�����|�C���g�̑I���Ƃ݂Ȃ��B
#define NEAR_DISTANCE_MAX			16

ImagingContext::ImagingContext()
{
	/*pass*/
}

/// �\�[�X�摜�ݒ�
void ImagingContext::setSrcImage(cv::Ptr<cv::Mat> pSrcImage)
{
	m_imagingCanvas.setSrcImage(pSrcImage);
	m_pSrcImage = pSrcImage;
}

/// �\�[�X�摜�Q��
cv::Ptr<cv::Mat> ImagingContext::refSrcImage()
{
	return m_pSrcImage;
}

/// �L�����o�X�ݒ�
bool ImagingContext::setupCanvas(const cv::Rect& srcArea, const cv::Size& dispSize)
{
	return m_imagingCanvas.setupCanvas(srcArea, dispSize);
}

/// �L�����o�X�Q��
cv::Mat& ImagingContext::refCanvas()
{
	return m_imagingCanvas.refCanvas();
}

/// ���W������
void ImagingContext::clearPointList()
{
	m_clickedPointList.clear();
}

/// �������W�񂪋󂩔ۂ���Ԃ��B
bool ImagingContext::isPointListEmpty() const
{
	return m_clickedPointList.empty();
}

/// �������W�̌����擾����B
int ImagingContext::pointListSize() const
{
	return m_clickedPointList.size();
}

/// �������W����擾����B
int ImagingContext::getPointList(std::vector<cv::Point>& points) const
{
	return m_clickedPointList.getPointList(points);
}

/// �������W�I��
bool ImagingContext::selectExistingPointIF(const int dispX, const int dispY)
{
	// ���W(dispX, dispY)���\�[�X�摜��̍��WsrcPt�ɕϊ�
	const cv::Point srcPt = m_imagingCanvas.convToSrcPoint(dispX, dispY);

	// srcPt�ɍł��߂����_������
	int nearestDist;
	cv::Point foundPt;
	const int selIdx = m_clickedPointList.selectFromExisting(srcPt, nearestDist, foundPt);
	if (selIdx < 0) {
		return false;
	}

	// �\���摜���NEAR_DISTANCE_MAX��f�ȓ�������
	int dispPtX, dispPtY;
	m_imagingCanvas.convToDispPoint(foundPt, dispPtX, dispPtY);
	if (!(std::abs(dispX - dispPtX) <= NEAR_DISTANCE_MAX && std::abs(dispY - dispPtY) <= NEAR_DISTANCE_MAX)) {
		return false;
	}

	m_clickedPointList.setCurIdx(selIdx);
	return true;
}

/// ���W�̒ǉ��܂��͈ړ�
void ImagingContext::addOrMovePoint(const int dispX, const int dispY)
{
	const cv::Point srcPt = m_imagingCanvas.convToSrcPoint(dispX, dispY);
	m_clickedPointList.addOrMovePoint(srcPt);
}

/// Current point�擾
bool ImagingContext::getCurPoint(cv::Point& curPt) const
{
	return m_clickedPointList.getCurPoint(curPt);
}

/// Current point�ړ�
void ImagingContext::moveCurPoint(const int dx, const int dy)
{
	m_clickedPointList.moveCurPoint(dx, dy);
}

/// Current point�؂�ւ�
void ImagingContext::changeCurrentPointToNext()
{
	m_clickedPointList.changeCurrentPointToNext();
}

/// �L�����o�X�ĕ`��
void ImagingContext::refreshCanvas()
{
	// �`��ς݂̃K�C�h������
	m_imagingCanvas.cleanup();

	// �`�揇�̒��_���X�gvertexes�擾
	const std::vector<cv::Point> vertexes = m_clickedPointList.getClockwizeLlist();

	// vertexes�̒�����current point����
	int curIdx = -1;
	cv::Point curPt;
	if (m_clickedPointList.getCurPoint(curPt)) {
		const int sz = (int)vertexes.size();
		for (int i = 0; i < sz; i++) {
			if (vertexes[i] == curPt) {
				curIdx = i;
				break;
			}
		}
	}

	// �`��
	m_imagingCanvas.drawPolylines(vertexes, NEAR_DISTANCE_MAX, curIdx);
}

/// �L�����o�X�ƃ\�[�X�摜��]
void ImagingContext::rotate(const int dir)
{
	// �\�[�X�摜90����]
	cv::Point ofsAfterRot;
	m_imagingCanvas.rotate(dir, ofsAfterRot);

	// �������W���X�g���e90����]
	m_clickedPointList.rotate(dir, ofsAfterRot);
}

/// �c�ݕ␳
bool ImagingContext::correctDistortion(const cv::Size& dstSz, cv::Mat& dstImg)
{
	const int nptsExp = 4;

	// �O���[�X�P�[���摜�ɕϊ�
	cv::Mat grayImage;
	if (!conv_color_to_gray(*m_pSrcImage, grayImage)) {
		return false;
	}

	// �������W(�c��ROI��4���_)�����v���̏��Ń��X�g��
	const std::vector<cv::Point> srcROICorners = m_clickedPointList.getClockwizeLlist();
	if (srcROICorners.size() != nptsExp) {
		return false;
	}

	// cv::Point2f�̃��X�g�ɕϊ�
	cv::Point2f srcROICorners2f[nptsExp];
	for (int i = 0; i < nptsExp; i++) {
		srcROICorners2f[i] = cv::Point2f((float)srcROICorners[i].x, (float)srcROICorners[i].y);
	}

	// �ϊ����s
	warp_image(grayImage, dstImg, srcROICorners2f, nptsExp, dstSz);

	return true;
}

/// �V�F�[�f�B���O�␳
bool ImagingContext::doShadingCorrection(const cv::Size& dstSz, cv::Mat& dstImg)
{
	cv::Mat tmp;

	// �p�[�X�y�N�e�B�u�␳
	cv::Mat gray1;
	if (!correctDistortion(dstSz, gray1)) {
		cout << __FUNCTION__ << "correctDistortion() failed." << endl;
		return false;
	}
	assert(gray1.channels() == 1);	// ���ʂ̓O���[�X�P�[���摜
#ifndef NDEBUG
	cv::imwrite(DBG_IMG_DIR "01_counter_warped_image.jpg", gray1);
#ifdef DBG_IMG_DIR
	cv::imwrite(DBG_IMG_DIR "01_counter_warped_image.jpg", gray1);
#endif
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
#ifndef NDEBUG
	cv::imshow("02_image_after_black_hat", gray2);
#ifdef DBG_IMG_DIR
	cv::imwrite(DBG_IMG_DIR "02_image_after_black_hat.jpg", gray2);
#endif
#endif

	// �ȉ��Agray2���ψꉻ�摜�ƌĂԁB

	// �ψꉻ�摜gray2������(gray1)
	cv::blur(gray2, gray1, cv::Size(3, 3));

	// ����������gray1�ɑ΂��A��Â̕��@��臒lth1���Z�o
	const double th1 = cv::threshold(gray1, tmp, 0, 255, cv::THRESH_OTSU);
	cout << "th1=" << th1 << endl;
	tmp.release();		// 2�l�����ʂ͎g��Ȃ�

#ifndef NDEBUG
	// get_unmasked_data()�̃e�X�g
	{
		cv::Mat nonmask = cv::Mat::ones(gray1.rows, gray1.cols, CV_8UC1);
		const std::vector<uchar> dbg_data = get_unmasked_data(gray1, nonmask);
		const int dbg_th1 = discriminant_analysis_by_otsu(dbg_data);
		cout << "dbg_th1=" << dbg_th1 << endl;
		if (dbg_th1 != (int)std::round(th1)) {
			throw std::logic_error("*** ERR ***");
		}
	}
#endif

	// �}�X�N�쐬
	// �������摜gray1�̋P�xth1�ȉ�����(0)�A���߂�(255)�ɂ���(mask)
	cv::Mat mask;
	cv::threshold(gray1, mask, th1, 255.0, cv::THRESH_BINARY);
#ifndef NDEBUG
	cv::imshow("03_mask", mask);
#ifdef DBG_IMG_DIR
	cv::imwrite(DBG_IMG_DIR "03_mask.jpg", mask);
#endif
#endif

	// �ψꉻ�摜gray2�̃}�X�N����Ȃ���f�f�[�^�𒊏o(data)
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
#ifndef NDEBUG
	cv::imshow("04_threshold_by_th3", gray2);
#ifdef DBG_IMG_DIR
	cv::imwrite(DBG_IMG_DIR "04_threshold_by_th3.jpg", gray2);
#endif
#endif

	// gray2�𔒍����](dstImg)
	cv::bitwise_not(gray2, dstImg);

	return true;
}
