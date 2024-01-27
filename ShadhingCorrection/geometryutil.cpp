#include "stdafx.h"
#include "geometryutil.h"

/// ベクトルの長さを取得する。
double get_vec_len(const cv::Point& v)
{
	return std::sqrt((double)v.x * (double)v.x + (double)v.y * (double)v.y);
}

/// 空の矩形か否か判定する。
bool is_empty_rect(const cv::Rect& rect)
{
	return (rect.width <= 0 || rect.height <= 0);
}

/// 座標を90°回転する。(座標系は左上原点前提)
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

/// 2点を含む矩形を作る。
cv::Rect make_outrect(const cv::Point& p1, const cv::Point& p2)
{
	const int sx = std::min(p1.x, p2.x);
	const int sy = std::min(p1.y, p2.y);
	const int lx = std::max(p1.x, p2.x);
	const int ly = std::max(p1.y, p2.y);
	return cv::Rect(sx, sy, lx - sx + 1, ly - sy + 1);
}

/// 矩形を90°回転する。(座標系は左上原点前提)
cv::Rect rotate_rect(const cv::Rect& rect, const int dir)
{
	const int sx = rect.x;
	const int sy = rect.y;
	const int lx = rect.x + rect.width - 1;
	const int ly = rect.y + rect.height - 1;

	const cv::Point spt2 = rotate_point(cv::Point(sx, sy), dir);
	const cv::Point lpt2 = rotate_point(cv::Point(lx, ly), dir);
	return make_outrect(spt2, lpt2);
}

/// 矩形のクリッピングを行う。
cv::Rect clip_rect_into_image(const cv::Rect& rect, const int width, const int height)
{
	const int sx = std::max(rect.x, 0);
	const int sy = std::max(rect.y, 0);
	const int ex = std::min(rect.x + rect.width, width);
	const int ey = std::min(rect.y + rect.height, height);

	return cv::Rect(sx, sy, ex - sx, ey - sy);
}

/// 矩形のクリッピングを行う。
cv::Rect clip_rect_into_rect(const cv::Rect& rect, const cv::Rect& clipRect)
{
	const int sx = std::max(rect.x, clipRect.x);
	const int sy = std::max(rect.y, clipRect.y);
	const int ex = std::min(rect.x + rect.width, clipRect.x + clipRect.width);
	const int ey = std::min(rect.y + rect.height, clipRect.y + clipRect.height);

	return cv::Rect(sx, sy, ex - sx, ey - sy);
}

/// Decompose rectangle into start point and end point.
void decompose_rect(const cv::Rect& rect, int& sx, int& sy, int& ex, int& ey)
{
	sx = rect.x;
	sy = rect.y;
	ex = sx + rect.width;
	ey = sy + rect.height;
}

/// Get scaled rectangle.
cv::Rect get_scaled_rect(const cv::Rect& rect, const double ratio)
{
	const cv::Size sz(rect.width, rect.height);
	cv::Rect r = get_scaled_rect_from_size(sz, ratio);
	r.x += rect.x;
	r.y += rect.y;
	return r;
}

/// Get scaled rectangle.
cv::Rect get_scaled_rect_from_size(const cv::Size& size, const double ratio)
{
	const int width = size.width;
	const int height = size.height;
	const int newWidth = (int)std::round(width * ratio);
	const int newHeight = (int)std::round(height * ratio);
	return cv::Rect((width - newWidth) / 2, (height - newHeight) / 2, newWidth, newHeight);
}
