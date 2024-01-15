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
