#pragma once
struct ClickedPointList
{
	std::vector<cv::Point2f> m_points;
	int m_curIdx;

	/*
		(NOTE)
		�\���͈͂̕��������̉�f���������̉摜�ɂ��Ẳ�]�ɂ���
		��]��̍��W�𐳊m�ɕ\�����߁Am_points�̗v�f��cv::Point�łȂ�
		cv::Point2f�ł���K�v������B
	*/

	ClickedPointList();

	void clear();

	/// (x, y)�ɍł��߂������v�f��T���B
	int selectFromExisting(const int x, const int y, int& nearestDist) const;

	/// ���W��I���܂��͒ǉ�����B
	void selectOrAdd(const int x, const int y);

	/// �ł����ォ�玞�v���̏��̃��X�g���擾����B
	std::vector<cv::Point2f> getClockwizeLlist() const;

	/// ���ݑI�𒆂̍��W���擾����B
	cv::Point2f getCurPoint() const;

	/// ���W��̊O�ڋ�`���擾����B
	bool getOutRect(cv::Rect2f& rect) const;

	/// �L�����o�X��90����]����B(���W�n�͍���n�O��)
	void rotate(const cv::Point2f& centerPt, const int dir);

private:
	/// �ł�����̍��W���擾����B
	static int get_most_topleft(const std::vector<cv::Point2f>& points);

	/// ���v���ŗׂ̍��W���擾����B
	static int get_clockwise_neighbor(const cv::Point2f& pt1, const std::vector<cv::Point2f>& points);

	/// �ł����ォ�玞�v���̏��̃��X�g���擾����B
	static std::vector<cv::Point2f> get_clockwize_list(std::vector<cv::Point2f>& points);

};
