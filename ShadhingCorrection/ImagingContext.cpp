#include "stdafx.h"
#include "ImagingCanvas.h"
#include "ClickedPointList.h"
#include "ImagingContext.h"

#include "imaging_op.h"
#include "geometryutil.h"
#include "geometry2futil.h"

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
	const std::vector<cv::Point2f> vertexes2f = m_clickedPointList.getClockwizeLlist();
	std::vector<cv::Point> vertexes;
	vertexes.reserve(vertexes2f.size());
	for (auto it = vertexes2f.begin(); it != vertexes2f.end(); it++) {
		vertexes.push_back(cv::Point((int)(it->x), (int)(it->y)));
	}
	m_imagingCanvas.drawPolylines(vertexes, NEAR_DISTANCE_MAX, thickness);
}

/// �L�����o�X�ƃ\�[�X�摜��]
void ImagingContext::rotate(const int dir)
{
	//const float cx = (float)m_srcArea.x + (float)m_srcArea.width / 2.0F;
	//const float cy = (float)m_srcArea.y + (float)m_srcArea.height / 2.0F;
	const float cx = (float)(m_imagingCanvas.getSrcImagePtr()->cols) / 2.0F;
	const float cy = (float)(m_imagingCanvas.getSrcImagePtr()->rows) / 2.0F;
	const cv::Point2f centerPt = cv::Point2f(cx, cy);
	cout << "centerPt=" << centerPt << endl;

	// �\�[�X�摜90����]
	// ����̓\�[�X�摜���S����]���Ƃ����]�ł���B
	m_imagingCanvas.rotate(dir);

	// �\�[�X�̈�90����]
	cv::Rect2f srcRect = m_srcArea;
	srcRect.x -= cx;
	srcRect.y -= cy;
	rotate_rect(srcRect, dir);
	srcRect.x += cx;
	srcRect.y += cy;
	m_srcArea = srcRect;

	// �������W���X�g���e90����]
	m_clickedPointList.rotate(centerPt, dir);
}

/// �c�ݕ␳
bool ImagingContext::correctDistortion(const double relWidth, const double relHeight, const int outputWidth)
{
	const std::vector<cv::Point2f> srcPts = m_clickedPointList.getClockwizeLlist();
	if (srcPts.size() != 4) {
		return false;
	}
	const int npts = (int)srcPts.size();

	cv::Mat dstImg;
	warp_image(*(m_imagingCanvas.getSrcImagePtr()), dstImg, &(srcPts[0]), npts, relWidth, relHeight, outputWidth);
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
