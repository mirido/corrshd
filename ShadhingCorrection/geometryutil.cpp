#include "stdafx.h"
#include "geometryutil.h"

/// ��̋�`���ۂ����肷��B
bool is_empty_rect(const cv::Rect& rect)
{
	return (rect.width <= 0 || rect.height <= 0);
}

/// ���W��90����]����B(���W�n�͍���n�O��)
cv::Point rotate_point(const cv::Point& pt, const int dir)
{
	cv::Point pt2;

	if (dir < 0) {
		pt2.x = pt.y;
		pt2.y = -pt.x;
	}
	else if (dir > 0) {
		pt2.x = -pt.y;
		pt2.y = pt.x;
	}
	else {
		pt2 = pt;
	}
	return pt2;
}

/// ��`��90����]����B(���W�n�͍���n�O��)
cv::Rect rotate_rect(const cv::Rect& rect, const int dir)
{
	const cv::Point spt(rect.x, rect.y);
	const cv::Point ept(rect.x + rect.width, rect.y + rect.height);

	const cv::Point spt2 = rotate_point(spt, dir);
	const cv::Point ept2 = rotate_point(ept, dir);

	return cv::Rect(spt.x, spt.y, ept.x - spt.x, ept.y - spt.x);
}

/// ��`�̃N���b�s���O���s���B
cv::Rect clip_rect_into_image(const cv::Rect& rect, const int width, const int height)
{
	const int sx = std::max(rect.x, 0);
	const int sy = std::max(rect.y, 0);
	const int ex = std::min(rect.x + rect.width, width);
	const int ey = std::min(rect.y + rect.height, height);

	return cv::Rect(sx, sy, ex - sx, ey - sy);
}

/// ��`�̃N���b�s���O���s���B
cv::Rect clip_rect_into_rect(const cv::Rect& rect, const cv::Rect& clipRect)
{
	const int sx = std::max(rect.x, clipRect.x);
	const int sy = std::max(rect.y, clipRect.y);
	const int ex = std::min(rect.x + rect.width, clipRect.x + clipRect.width);
	const int ey = std::min(rect.y + rect.height, clipRect.y + clipRect.height);

	return cv::Rect(sx, sy, ex - sx, ey - sy);
}
