#pragma once

/// �x�N�g���̒������擾����B
double get_vec_len(const cv::Point2f& v);

/// ��̋�`���ۂ����肷��B
bool is_empty_rect(const cv::Rect2f& rect);

/// ���W��90����]����B(���W�n�͍���n�O��)
cv::Point2f rotate_point(const cv::Point2f& pt, const int dir);

/// ��`��90����]����B(���W�n�͍���n�O��)
cv::Rect2f rotate_rect(const cv::Rect2f& rect, const int dir);

/// ��`�̃N���b�s���O���s���B
cv::Rect2f clip_rect_into_image(const cv::Rect2f& rect, const int width, const int height);

/// ��`�̃N���b�s���O���s���B
cv::Rect2f clip_rect_into_rect(const cv::Rect2f& rect, const cv::Rect2f& clipRect);
