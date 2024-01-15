#include "stdafx.h"
#include "ImagingCanvas.h"

#include "geometryutil.h"
#include "imaging_op.h"

// [CONF] �K�C�h���̕`��F
#define GUIDE_COLOR		cv::Scalar(0, 0, 255)

ImagingCanvas::ImagingCanvas()
{
	/*pass*/
}

/// �\�[�X�摜��ݒ肷��B
void ImagingCanvas::setSrcImage(cv::Ptr<cv::Mat> pSrcImage)
{
	m_pSrcImage = pSrcImage;
}

/// �L�����o�X���X�V����B
bool ImagingCanvas::setupCanvas(const cv::Rect& srcArea, const cv::Size& dispSize)
{
	m_srcArea = srcArea;
	m_dispSize = dispSize;

	// �\�[�X�摜��m_srcArea���̂ݐ؂蔲��
	cv::Mat ROISrc = cv::Mat(*m_pSrcImage, m_srcArea);
	cv::Mat resized;
	cv::resize(ROISrc, resized, m_dispSize);

	// BGR�摜�ɕϊ�
	if (!conv_color_to_BGR(resized, m_canvas)) {
		return false;
	}

	// �ł����L�����o�X��PolyLine�����p�ɋL��
	m_canvasMaster = m_canvas.clone();

	// Dirty area�N���A
	m_dirtyArea = cv::Rect();

	return true;
}

/// �}�E�X�N���b�N�ʒu���\�[�X�摜��̍��W�ɕϊ�����B
cv::Point ImagingCanvas::convToSrcPoint(const int dispX, const int dispY)
{
	const int srcX = m_srcArea.x + (dispX * m_srcArea.width) / m_dispSize.width;
	const int srcY = m_srcArea.y + (dispY * m_srcArea.height) / m_dispSize.height;
	return cv::Point(srcX, srcY);
}

/// �\�[�X�摜��̍��W���}�E�X�N���b�N�ʒu�ɕϊ�����B
void ImagingCanvas::convToDispPoint(const cv::Point& srcPt, int& dispX, int& dispY)
{
	dispX = (srcPt.x * m_dispSize.width) / m_srcArea.width - m_srcArea.x;
	dispY = (srcPt.y * m_dispSize.height) / m_srcArea.height - m_srcArea.y;
}

/// �L�����o�X�ɑ��p�`��`�悷��B
void ImagingCanvas::drawPolylines(const std::vector<cv::Point>& vertexes, const int vtxMarkerRadius, const int curIdx)
{
	if (vertexes.empty()) {
		return;
	}

	// �L�����o�X��̍��W�ɕϊ�
	const auto vtxs = convToCanvasPointInBulk(vertexes);

	// ���p�`�`��
	int npts[1];
	npts[0] = (int)vtxs.size();
	const cv::Point* ppts[1];
	ppts[0] = &(vtxs[0]);
	if (npts[0] >= 2) {
		cv::polylines(m_canvas, ppts, npts, 1, true, GUIDE_COLOR, 1);
	}

	// ���_�}�[�J�[�`��
	int sxMin = vtxs.front().x;
	int syMin = vtxs.front().y;
	int exMax = vtxs.front().x;
	int eyMax = vtxs.front().y;
	for (auto it = vtxs.begin(); it != vtxs.end(); it++) {
		const int sx = it->x - vtxMarkerRadius;
		const int ex = sx + 2 * vtxMarkerRadius;
		const int sy = it->y - vtxMarkerRadius;
		const int ey = sy + 2 * vtxMarkerRadius;

		const int thickness = (it - vtxs.begin() == (ptrdiff_t)curIdx) ? 3 : 1;
		cv::rectangle(m_canvas, cv::Point(sx, sy), cv::Point(ex, ey), GUIDE_COLOR, thickness);

		sxMin = std::min(sxMin, sx - (vtxMarkerRadius + thickness));
		syMin = std::min(syMin, sy - (vtxMarkerRadius + thickness));
		exMax = std::max(exMax, ex + (vtxMarkerRadius + thickness));
		eyMax = std::max(eyMax, ey + (vtxMarkerRadius + thickness));
	}

	// Dirty area�L��
	sxMin = std::max(sxMin, 0);
	syMin = std::max(syMin, 0);
	exMax = std::min(exMax, m_canvas.cols);
	eyMax = std::min(eyMax, m_canvas.rows);
	const cv::Rect rect = cv::Rect(sxMin, syMin, exMax - sxMin, eyMax - syMin);
	m_dirtyArea = clip_rect_into_image(rect, m_canvas.cols, m_canvas.rows);
}

/// �L�����o�X�ւ̕`�����������B
void ImagingCanvas::cleanup()
{
	const cv::Rect dirtyRect = m_dirtyArea;

	if (!is_empty_rect(dirtyRect)) {
		cv::Mat ROISrc = cv::Mat(m_canvasMaster, dirtyRect);
		cv::Mat ROIDst = cv::Mat(m_canvas, dirtyRect);
		//cv::rectangle(ROISrc, cv::Point(0, 0), cv::Point(ROISrc.cols - 1, ROISrc.rows - 1), cv::Scalar(255, 0, 0), 5);		// DEBUG
		ROISrc.copyTo(ROIDst);
	}
}

/// �L�����o�X��90����]����B(���W�n�͍��㌴�_�O��)
void ImagingCanvas::rotate(const int dir, cv::Point& ofsAfterRot)
{
	if (dir == 0) {
		ofsAfterRot = cv::Point(0, 0);
		return;
	}

	// �\�[�X�摜��]
	const auto dirFlag = (dir < 0) ? cv::ROTATE_90_COUNTERCLOCKWISE : cv::ROTATE_90_CLOCKWISE;
	cv::Mat dstImg;
	cv::rotate(*m_pSrcImage, dstImg, dirFlag);
	*m_pSrcImage = dstImg;

	// ��]���ʂ��1�ی��ɕ��s�ړ�����I�t�Z�b�gofsAfterRot
	if (dir < 0) {
		// (���㌴�_�Ŕ����v���)
		// ��4�ی�����ړ�����I�t�Z�b�g��ݒ�
		ofsAfterRot = cv::Point(0, m_pSrcImage->rows - 1);
	}
	else {
		// (���㌴�_�Ŏ��v���)
		// ��2�ی�����ړ�����I�t�Z�b�g��ݒ�
		ofsAfterRot = cv::Point(m_pSrcImage->cols - 1, 0);
	}
	/*
		(NOTE)
		OpenCV���܂߁A�摜��̍��W�n�͈�ʂɍ��㌴�_�ł���B
		���̂��߁A���_�����L����̈�
			O --- A
			|     |
			C --- B
		���摜�ƂƂ���90����]�������ꍇ�A
		�����v���Ȃ��]���A�����_(0, 0)�Ɉڂ����s�ړ��A
		���v���Ȃ��]���C�����_(0, 0)�Ɉڂ����s�ړ�
		����]��ɂ��ꂼ��s�Ȃ��K�v������B
		(�����Ȃ��ƁA��]���ʂ̍��オ���_�ɂȂ�Ȃ��B)
		��̃I�t�Z�b�gofsAfterRot�͂��̕��s�ړ���\���B
	*/

	// �\�[�X�̈�90����]
	// �摜�Ɠ�������]���o�Ă���1�ی��Ƃ���B
	m_srcArea = rotate_rect(m_srcArea, dir);
	m_srcArea.x += ofsAfterRot.x;
	m_srcArea.y += ofsAfterRot.y;

	// �\���T�C�Y�̏c������ւ�
	std::swap(m_dispSize.width, m_dispSize.height);

	// �L�����o�X�X�V
	setupCanvas(m_srcArea, m_dispSize);
}

/// �L�����o�X���Q�Ƃ���B
cv::Mat& ImagingCanvas::refCanvas()
{
	return m_canvas;
}

//
//	Private
//

/// �\�[�X�摜��̍��W���L�����o�X��̍��W�ɕϊ�
cv::Point ImagingCanvas::convToCanvasPoint(const cv::Point& point) const
{
	cv::Point dstPt(point);

	dstPt.x -= m_srcArea.x;
	dstPt.y -= m_srcArea.y;

	dstPt.x = (dstPt.x * m_dispSize.width) / m_srcArea.width;
	dstPt.y = (dstPt.y * m_dispSize.height) / m_srcArea.height;

	return dstPt;
}

/// �\�[�X�摜��̍��W���L�����o�X��̍��W�Ɉꊇ�ϊ�
std::vector<cv::Point> ImagingCanvas::convToCanvasPointInBulk(const std::vector<cv::Point>& points) const
{
	const size_t sz = points.size();

	std::vector<cv::Point> dst(sz);
	for (size_t i = 0; i < sz; i++) {
		dst[i] = convToCanvasPoint(points[i]);
	}

	return dst;
}
