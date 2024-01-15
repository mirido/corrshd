#include "stdafx.h"
#include "ImagingCanvas.h"
#include "ClickedPointList.h"
#include "ImagingContext.h"

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
	const int thickness = std::min(1, m_srcArea.width / m_dispWidth);

	// �`��ς݂̃K�C�h������
	m_imagingCanvas.cleanup();

	// �K�C�h���`��
	const std::vector<cv::Point> vertexes = m_clickedPointList.getClockwizeLlist();
	m_imagingCanvas.drawPolylines(vertexes, NEAR_DISTANCE_MAX, thickness);
}

/// �L�����o�X�ƃ\�[�X�摜��]
void ImagingContext::rotate(const int dir)
{
	m_imagingCanvas.rotate(dir);
	m_clickedPointList.rotate(dir);
}

/// �c�ݕ␳
bool ImagingContext::correctDistortion(const double relWidth, const double relHeight, const int outputWidth)
{
	const std::vector<cv::Point> vertexes = m_clickedPointList.getClockwizeLlist();
	if (vertexes.size() != 4) {
		return false;
	}

	// �ϊ��O��4�_
	cv::Point2f srcPts[4];
	for (int i = 0; i < 4; i++) {
		srcPts[i].x = (float)vertexes[i].x;
		srcPts[i].y = (float)vertexes[i].y;
	}

	// �ϊ����4�_
	const int nOutWidth = outputWidth;
	const int nOutHeight = (int)std::round(((double)outputWidth * relHeight) / relWidth);
	const float left = (float)nOutWidth;
	const float btm = (float)nOutHeight;
	cv::Point2f dstPts[4];
	dstPts[0] = cv::Point2f(0.0F, 0.0F);
	dstPts[1] = cv::Point2f(left, 0.0F);
	dstPts[2] = cv::Point2f(left, btm);
	dstPts[3] = cv::Point2f(0.0f, btm);

	// �ϊ��s��擾
	const cv::Mat M = cv::getPerspectiveTransform(srcPts, dstPts);

	// �ϊ����{
	cv::Ptr<cv::Mat> pTransformed;
	cv::warpPerspective(*(m_imagingCanvas.getSrcImagePtr()), *pTransformed, M, cv::Size(nOutWidth, nOutHeight));

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
