#pragma once

/// 空の矩形か否か判定する。
bool is_empty_rect(const cv::Rect& rect);

/// 座標を90°回転する。(座標系は左手系前提)
cv::Point rotate_point(const cv::Point& pt, const int dir);

/// 矩形を90°回転する。(座標系は左手系前提)
cv::Rect rotate_rect(const cv::Rect& rect, const int dir);

/// 矩形のクリッピングを行う。
cv::Rect clip_rect_into_image(const cv::Rect& rect, const int width, const int height);

/// 矩形のクリッピングを行う。
cv::Rect clip_rect_into_rect(const cv::Rect& rect, const cv::Rect& clipRect);
