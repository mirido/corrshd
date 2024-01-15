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
	: m_dispWidth(0)
{
	/*pass*/
}

/// �L�����o�X�ݒ�
void ImagingContext::setSrcImage(cv::Ptr<cv::Mat> pSrcImage)
{
	m_imagingCanvas.setSrcImage(pSrcImage);
	m_srcArea = cv::Rect(0, 0, pSrcImage->cols, pSrcImage->rows);
	m_dispWidth = pSrcImage->cols;
}

/// �\���d�l�ݒ�
void ImagingContext::setDispGeometry(const cv::Rect& srcArea, const int dispWidth)
{
	m_srcArea = srcArea;
	m_dispWidth = dispWidth;
}

/// ���W������
void ImagingContext::clearPointList()
{
	m_clickedPointList.clear();
}

/// ���W�ǉ�
void ImagingContext::selectOrAdd(const int x, const int y)
{
	const int srcWidth = m_srcArea.width;
	const int srcX = m_srcArea.x + (x * srcWidth) / m_dispWidth;
	const int srcY = m_srcArea.y + (y * srcWidth) / m_dispWidth;

	m_clickedPointList.selectOrAdd(srcX, srcY);
}

/// �L�����o�X�X�V
void ImagingContext::refreshCanvas()
{
	const double magToDisp = (double)m_srcArea.width / (double)m_dispWidth;

	// �`��ς݂̃K�C�h������
	m_imagingCanvas.cleanup();

	// �K�C�h���`��
	const std::vector<cv::Point> vertexes = m_clickedPointList.getClockwizeLlist();
	m_imagingCanvas.drawPolylines(vertexes, NEAR_DISTANCE_MAX, magToDisp);
}

/// �L�����o�X�ƃ\�[�X�摜��]
void ImagingContext::rotate(const int dir)
{
	// �\�[�X�摜90����]
	// �摜�͉�]���o�Ă���1�ی��ɂ��葱����B
	cv::Point ofsAfterRot;
	m_imagingCanvas.rotate(dir, ofsAfterRot);

	// �\�[�X�̈�90����]
	// �摜�Ɠ�������]���o�Ă���1�ی��Ƃ���B
	rotate_rect(m_srcArea, dir);
	m_srcArea.x += ofsAfterRot.x;
	m_srcArea.y += ofsAfterRot.y;

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
	warp_image(*(m_imagingCanvas.getSrcImagePtr()), dstImg, srcPts2f, nptsExp, relWidth, relHeight, outputWidth);
	*(m_imagingCanvas.getSrcImagePtr()) = dstImg;
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

/// �L�����o�X���Q�Ƃ���B
cv::Mat& ImagingContext::refCanvas()
{
	return m_imagingCanvas.refCanvas();
}

/// Current point���擾����B
bool ImagingContext::getCurPt(cv::Point& pt) const
{
	const int curIdx = m_clickedPointList.m_curIdx;

	if (curIdx < 0) {
		pt = cv::Point();
		return false;
	}
	else {
		assert(0 <= curIdx && (size_t)curIdx < m_clickedPointList.m_points.size());
		pt = m_clickedPointList.m_points[curIdx];
		return true;
	}
}
