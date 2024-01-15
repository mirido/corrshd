#pragma once
class ClickedPointList
{
	std::vector<cv::Point> m_points;
	int m_curIdx;

public:
	ClickedPointList();

	void clear();

	/// �������W�񂪋󂩔ۂ���Ԃ��B
	bool empty() const;

	/// �������W�̌����擾����B
	int size() const;

	/// �������W����擾����B
	int getPointList(std::vector<cv::Point>& points) const;

	/// �w����W�ɍł��߂��������W��I������B
	int selectFromExisting(const cv::Point& srcPt, int& nearestDist, cv::Point& foundPt) const;

	/// ���W��ǉ��܂��͈ړ�����B
	void addOrMovePoint(const cv::Point& srcPt);

	/// �ł����ォ�玞�v���̏��̃��X�g���擾����B
	std::vector<cv::Point> getClockwizeLlist() const;

	/// Cureent index��ݒ肷��B
	void setCurIdx(const int idx);

	/// Current point���ړ�����B
	bool moveCurPoint(const int dx, const int dy);

	/// Current point������I�ɐ؂�ւ���B
	void changeCurrentPointToNext();

	/// Current point���擾����B
	bool getCurPoint(cv::Point& curPt) const;

	/// ���W��̊O�ڋ�`���擾����B
	bool getOutRect(cv::Rect& rect) const;

	/// �L�����o�X��90����]����B(���W�n�͍��㌴�_�O��)
	void rotate(const int dir, const cv::Point& ofsAfterRot);

private:
	/// �ł�����̍��W���擾����B
	static int get_most_topleft(const std::vector<cv::Point>& points);

	/// ���v���ŗׂ̍��W���擾����B
	static int get_clockwise_neighbor(const cv::Point& pt1, const std::vector<cv::Point>& points);

	/// �ł����ォ�玞�v���̏��̃��X�g���擾����B
	static std::vector<cv::Point> get_clockwize_list(std::vector<cv::Point>& points);

};
