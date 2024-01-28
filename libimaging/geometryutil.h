#pragma once

/// �x�N�g���̒������擾����B
double get_vec_len(const cv::Point& v);

/// ��̋�`���ۂ����肷��B
bool is_empty_rect(const cv::Rect& rect);

/// ���W��90����]����B(���W�n�͍��㌴�_�O��)
cv::Point rotate_point(const cv::Point& pt, const int dir);

/// ���W��90����]����B(���W�n�͍��㌴�_�O��)
cv::Point rotate_point(const cv::Point& pt, const int dir);

/// ��`��90����]����B(���W�n�͍��㌴�_�O��)
cv::Rect rotate_rect(const cv::Rect& rect, const int dir);

/// ��`�̃N���b�s���O���s���B
cv::Rect clip_rect_into_image(const cv::Rect& rect, const int width, const int height);

/// ��`�̃N���b�s���O���s���B
cv::Rect clip_rect_into_rect(const cv::Rect& rect, const cv::Rect& clipRect);

/// Decompose rectangle into start point and end point.
void decompose_rect(const cv::Rect& rect, int& sx, int& sy, int& ex, int& ey);

/// Decompose rectangle into start point and end point.
void decompose_rect(const cv::Rect& rect, int& sx, int& sy, int& ex, int& ey);

/// Get scaled rectangle.
cv::Rect get_scaled_rect(const cv::Rect& rect, const double ratio);

/// Get scaled rectangle.
cv::Rect get_scaled_rect_from_size(const cv::Size& size, const double ratio);
