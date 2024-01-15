#include "stdafx.h"
#include "geometry2futil.h"

/// �x�N�g���̒������擾����B
double get_vec_len(const cv::Point2f& v)
{
	return std::sqrt((double)v.x * (double)v.x + (double)v.y * (double)v.y);
}

/// ��̋�`���ۂ����肷��B
bool is_empty_rect(const cv::Rect2f& rect)
{
	return (rect.width <= 0 || rect.height <= 0);
}

/// ���W��90����]����B(���W�n�͍���n�O��)
cv::Point2f rotate_point(const cv::Point2f& pt, const int dir)
{
	cv::Point2f pt2;

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
cv::Rect2f rotate_rect(const cv::Rect2f& rect, const int dir)
{
	const cv::Point2f spt(rect.x, rect.y);
	const cv::Point2f ept(rect.x + rect.width, rect.y + rect.height);

	const cv::Point2f spt2 = rotate_point(spt, dir);
	const cv::Point2f ept2 = rotate_point(ept, dir);

	return cv::Rect2f(spt.x, spt.y, ept.x - spt.x, ept.y - spt.x);
}

/// ��`�̃N���b�s���O���s���B
cv::Rect2f clip_rect_into_image(const cv::Rect2f& rect, const int width, const int height)
{
	const float sx = std::max(rect.x, 0.0F);
	const float sy = std::max(rect.y, 0.0F);
	const float ex = std::min(rect.x + rect.width, (float)width);
	const float ey = std::min(rect.y + rect.height, (float)height);

	return cv::Rect2f(sx, sy, ex - sx, ey - sy);
}

/// ��`�̃N���b�s���O���s���B
cv::Rect2f clip_rect_into_rect(const cv::Rect2f& rect, const cv::Rect2f& clipRect)
{
	const float sx = std::max(rect.x, clipRect.x);
	const float sy = std::max(rect.y, clipRect.y);
	const float ex = std::min(rect.x + rect.width, clipRect.x + clipRect.width);
	const float ey = std::min(rect.y + rect.height, clipRect.y + clipRect.height);

	return cv::Rect2f(sx, sy, ex - sx, ey - sy);
}
