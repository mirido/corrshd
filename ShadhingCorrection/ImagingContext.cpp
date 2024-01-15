#include "stdafx.h"
#include "ImagingCanvas.h"
#include "ClickedPointList.h"
#include "ImagingContext.h"

#include "imaging_op.h"
#include "geometryutil.h"

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

/// �L�����o�X�X�V
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

/// Current point�ύX
bool ImagingContext::selectExistingPointIF(const int dispX, const int dispY)
{
	const cv::Point srcPt = m_imagingCanvas.convToSrcPoint(dispX, dispY);

	int nearestDist;
	const int selIdx = m_clickedPointList.selectFromExisting(srcPt, nearestDist);
	if (selIdx < 0 || nearestDist > NEAR_DISTANCE_MAX) {
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
bool ImagingContext::correctDistortion(const double relWidth, const double relHeight, const int outputWidth)
{
	const int nptsExp = 4;

	const std::vector<cv::Point> srcPts = m_clickedPointList.getClockwizeLlist();
	if (srcPts.size() != nptsExp) {
		return false;
	}

	cv::Point2f srcPts2f[nptsExp];
	for (int i = 0; i < nptsExp; i++) {
		srcPts2f[i] = cv::Point2f((float)srcPts[i].x, (float)srcPts[i].y);
	}

	cv::Mat dstImg;
	warp_image(*m_pSrcImage, dstImg, srcPts2f, nptsExp, relWidth, relHeight, outputWidth);
	*m_pSrcImage = dstImg;
	return true;
}

/// �؂蔲��
bool ImagingContext::cutOff()
{
	return false;
}

/// �V�F�[�f�B���O�␳
bool ImagingContext::shadingCorrection()
{
	return false;
}

/// �q�X�g�O�����ϓ���
bool ImagingContext::equalizeHist()
{
	return false;
}
