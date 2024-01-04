#pragma once

/// ベクトルの長さを取得する。
double get_vec_len(const cv::Point2f& v);

/// 空の矩形か否か判定する。
bool is_empty_rect(const cv::Rect2f& rect);

/// 座標を90°回転する。(座標系は左手系前提)
cv::Point2f rotate_point(const cv::Point2f& pt, const int dir);

/// 矩形を90°回転する。(座標系は左手系前提)
cv::Rect2f rotate_rect(const cv::Rect2f& rect, const int dir);

/// 矩形のクリッピングを行う。
cv::Rect2f clip_rect_into_image(const cv::Rect2f& rect, const int width, const int height);

/// 矩形のクリッピングを行う。
cv::Rect2f clip_rect_into_rect(const cv::Rect2f& rect, const cv::Rect2f& clipRect);
