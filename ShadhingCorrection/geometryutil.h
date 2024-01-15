#pragma once

/// ��̋�`���ۂ����肷��B
bool is_empty_rect(const cv::Rect& rect);

/// ���W��90����]����B(���W�n�͍���n�O��)
cv::Point rotate_point(const cv::Point& pt, const int dir);

/// ��`��90����]����B(���W�n�͍���n�O��)
cv::Rect rotate_rect(const cv::Rect& rect, const int dir);

/// ��`�̃N���b�s���O���s���B
cv::Rect clip_rect_into_image(const cv::Rect& rect, const int width, const int height);

/// ��`�̃N���b�s���O���s���B
cv::Rect clip_rect_into_rect(const cv::Rect& rect, const cv::Rect& clipRect);
