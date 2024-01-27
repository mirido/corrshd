#pragma once

/// ベクトルの長さを取得する。
double get_vec_len(const cv::Point& v);

/// 空の矩形か否か判定する。
bool is_empty_rect(const cv::Rect& rect);

/// 座標を90°回転する。(座標系は左上原点前提)
cv::Point rotate_point(const cv::Point& pt, const int dir);

/// 座標を90°回転する。(座標系は左上原点前提)
cv::Point rotate_point(const cv::Point& pt, const int dir);

/// 矩形を90°回転する。(座標系は左上原点前提)
cv::Rect rotate_rect(const cv::Rect& rect, const int dir);

/// 矩形のクリッピングを行う。
cv::Rect clip_rect_into_image(const cv::Rect& rect, const int width, const int height);

/// 矩形のクリッピングを行う。
cv::Rect clip_rect_into_rect(const cv::Rect& rect, const cv::Rect& clipRect);

/// Decompose rectangle into start point and end point.
void decompose_rect(const cv::Rect& rect, int& sx, int& sy, int& ex, int& ey);

/// Decompose rectangle into start point and end point.
void decompose_rect(const cv::Rect& rect, int& sx, int& sy, int& ex, int& ey);

/// Get scaled rectangle.
cv::Rect get_scaled_rect(const cv::Rect& rect, const double ratio);

/// Get scaled rectangle.
cv::Rect get_scaled_rect_from_size(const cv::Size& size, const double ratio);
