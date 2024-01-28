#include "stdafx.h"
#include "ClickedPointList.h"
#include "ImagingCanvas.h"
#include "IImgFunc.h"
#include "ImagingContext.h"

#include "ImgFuncBase.h"
#include "ImgFunc_shdc01.h"
#include "ImgFunc_shdc02.h"

#include "imaging_op.h"

// [CONF] �N���b�N�ʒu�̋�����臒l
// �����|�C���g�Ƃ̃}���n�b�^���������ȉ��̒l�ȉ��Ȃ�����|�C���g�̑I���Ƃ݂Ȃ��B
#define NEAR_DISTANCE_MAX			16

ImagingContext::ImagingContext()
{
	// Select algorithm.
	// TODO: Make it variable by command line arguments.
	//m_pImgFunc = std::unique_ptr<IImgFunc>(new ImgFunc_shdc01);
	m_pImgFunc = std::unique_ptr<IImgFunc>(new ImgFunc_shdc02);
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

/// �ł����ォ�玞�v���̏��̃��X�g���擾����B
std::vector<cv::Point> ImagingContext::getClockwiseList() const
{
	return m_clickedPointList.getClockwiseLlist();
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
	const std::vector<cv::Point> vertexes = m_clickedPointList.getClockwiseLlist();

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
	const std::vector<cv::Point> srcROICorners = m_clickedPointList.getClockwiseLlist();
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

#ifndef NDEBUG
	cout << std::setbase(10);
#endif

	m_pImgFunc->resetImgDumpCnt();

	// Perspective correction
	cv::Mat gray1;
	if (!correctDistortion(dstSz, gray1)) {
		cout << __FUNCTION__ << "correctDistortion() failed." << endl;
		return false;
	}
	assert(gray1.channels() == 1);	// ���ʂ̓O���[�X�P�[���摜
	m_pImgFunc->dumpImg(gray1, "counter_warped_image", DBG_IMG_DIR);

	// Image processing after perspective correction
	return m_pImgFunc->run(gray1, dstImg);
}
