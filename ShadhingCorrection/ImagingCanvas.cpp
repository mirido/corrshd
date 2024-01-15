#include "stdafx.h"
#include "ImagingCanvas.h"

#include "geometryutil.h"

// [CONF] �K�C�h���̕`��F
#define GUIDE_COLOR		cv::Scalar(0, 0, 255)

ImagingCanvas::ImagingCanvas()
{
	/*pass*/
}

/// �\�[�X�摜��ݒ肷��B
bool ImagingCanvas::setSrcImage(cv::Ptr<cv::Mat> pSrcImage)
{
	// �p�����[�^�L��
	m_pSrcImage = pSrcImage;

	switch (m_pSrcImage->channels()) {
	case 3:
		m_pSrcImage->copyTo(m_canvas);
		break;
	case 1:
		cv::cvtColor(*m_pSrcImage, m_canvas, cv::COLOR_GRAY2BGR);
		break;
	default:
		return false;
	}

	// Dirty area�N���A
	m_dirtyArea = cv::Rect();

	return true;
}

/// �L�����o�X�ɑ��p�`��`�悷��B
void ImagingCanvas::drawPolylines(const std::vector<cv::Point>& vertexes, const int vtxMarkerRadius_, const double magToDisp)
{
	const int thickness = std::max(1, (int)std::round(magToDisp));
		
	if (vertexes.empty()) {
		return;
	}

	// ���p�`�\��
	int npts[1];
	npts[0] = (int)vertexes.size();
	const cv::Point* ppts[1];
	ppts[0] = &(vertexes[0]);
	if (npts[0] >= 2) {
		cv::polylines(m_canvas, ppts, npts, 1, true, GUIDE_COLOR, thickness);
	}

	// ���_�}�[�J�[�`��
	const int vtxMarkerRadius = (int)std::round(vtxMarkerRadius_ * magToDisp);
	int sxMin = vertexes.front().x;
	int syMin = vertexes.front().y;
	int exMax = vertexes.front().x;
	int eyMax = vertexes.front().y;
	for (auto it = vertexes.begin(); it != vertexes.end(); it++) {
		const int sx = it->x - vtxMarkerRadius;
		const int ex = sx + 2 * vtxMarkerRadius;
		const int sy = it->y - vtxMarkerRadius;
		const int ey = sy + 2 * vtxMarkerRadius;

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
		cv::Mat ROISrc = cv::Mat(*m_pSrcImage, dirtyRect);
		cv::Mat ROIDst = cv::Mat(m_canvas, dirtyRect);
		//scv::rectangle(ROISrc, cv::Point(0, 0), cv::Point(ROISrc.cols - 1, ROISrc.rows - 1), cv::Scalar(255, 0, 0), 5);		// DEBUG
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
		ofsAfterRot = cv::Point(0, m_pSrcImage->rows);
	}
	else {
		// (���㌴�_�Ŏ��v���)
		// ��2�ی�����ړ�����I�t�Z�b�g��ݒ�
		ofsAfterRot = cv::Point(m_pSrcImage->cols, 0);
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

	// �L�����o�X�Đݒ�
	setSrcImage(m_pSrcImage);
}

/// �\�[�X�摜���Q�Ƃ���B
cv::Ptr<cv::Mat> ImagingCanvas::getSrcImagePtr()
{
	return m_pSrcImage;
}

/// �L�����o�X���Q�Ƃ���B
cv::Mat& ImagingCanvas::refCanvas()
{
	return m_canvas;
}
