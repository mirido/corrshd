#include "stdafx.h"
#include "ClickedPointList.h"

#include "geometryutil.h"

// [CONF] �I������_�̐�
#define NPOINTS_MAX					4

namespace
{
	/// �x�N�g���̒������擾����B
	double get_vec_len(const cv::Point& v)
	{
		return std::sqrt((double)v.x * (double)v.x + (double)v.y * (double)v.y);
	}

}	// namespace

ClickedPointList::ClickedPointList()
	: m_curIdx(-1)
{
	/*pass*/
}

void ClickedPointList::clear()
{
	m_points.clear();
	m_curIdx = -1;
}

/// (x, y)�ɍł��߂������v�f��T���B
int ClickedPointList::selectFromExisting(const int x, const int y, int& nearestDist) const
{
	const int sz = (int)m_points.size();
	nearestDist = 0;
	int nearestIdx = -1;
	for (int i = 0; i < sz; i++) {
		const cv::Point& pt = m_points[i];		// Alias
		const int dist = std::min(std::abs(x - pt.x), std::abs(y - pt.y));
		if (nearestIdx < 0 || dist < nearestDist) {
			nearestDist = dist;
			nearestIdx = i;
		}
	}

	// �������`�F�b�N
	// �ł��߂������v�f��������Ȃ��̂�m_points����̂Ƃ��݂̂̂͂��B
	if (nearestIdx < 0) {
		if (!m_points.empty()) {
			throw std::logic_error("*** ERR ***");
		}
	}

	return nearestIdx;
}

/// ���W��I���܂��͒ǉ�����B
void ClickedPointList::selectOrAdd(const int x, const int y)
{
	int nearestDist;
	const int foundIdx = selectFromExisting(x, y, nearestDist);
	if (foundIdx < 0 || (nearestDist > 0 && m_points.size() < NPOINTS_MAX)) {
		// (�����v�f��������Ȃ��������A
		//  �����̂ǂ�Ƃ��قȂ���W���I������A�����W�̐����ő吔�ɒB���Ă��Ȃ�)
		// �ǉ�
		m_points.push_back(cv::Point(x, y));
		m_curIdx = (int)(m_points.size() - 1);
	}
	else {
		// �ł��߂������v�f��I��
		assert(0 <= foundIdx && (size_t)foundIdx < m_points.size());
		m_points[foundIdx] = cv::Point(x, y);
		m_curIdx = foundIdx;
	}
}

/// �ł����ォ�玞�v���̏��̃��X�g���擾����B
std::vector<cv::Point> ClickedPointList::getClockwizeLlist() const
{
	std::vector<cv::Point> points(m_points);
	return get_clockwize_list(points);
}

/// ���ݑI�𒆂̍��W���擾����B
cv::Point ClickedPointList::getCurPoint() const
{
	if (m_curIdx < 0) {
		// (�������W����)
		throw std::logic_error("*** ERR ***");
	}
	assert(0 <= m_curIdx && m_curIdx < (size_t)m_points.size());
	return m_points[m_curIdx];
}

/// ���W��̊O�ڋ�`���擾����B
bool ClickedPointList::getOutRect(cv::Rect& rect) const
{
	rect = cv::Rect();

	if (m_points.empty()) {
		return false;
	}

	int sx, sy, ex, ey;
	sx = ex = m_points.front().x;
	sy = ey = m_points.front().y;
	const int sz = (int)m_points.size();
	for (int i = 1; i < sz; i++) {
		const cv::Point& pt = m_points[i];		// Alias
		if (pt.x < sx) {
			sx = pt.x;
		}
		else if (ex <= pt.x) {
			ex = pt.x + 1;
		}
		else {
			/*pass*/
		}
		if (pt.y < sy) {
			sy = pt.y;
		}
		else if (ey <= pt.y) {
			ey = pt.y + 1;
		}
		else {
			/*pass*/
		}
	}
	if (ex - sx <= 0 || ey - sy <= 0) {
		return false;
	}

	rect.x = sx;
	rect.y = sy;
	rect.width = ex - sx;
	rect.height = ey - sy;

	return true;
}

/// �L�����o�X��90����]����B(���W�n�͍���n�O��)
void ClickedPointList::rotate(const int dir)
{
	for (auto it = m_points.begin(); it != m_points.end(); it++) {
		rotate_point(*it, dir);
	}
}

//
//	Private
//

/// �ł�����̍��W���擾����B
int ClickedPointList::get_most_topleft(const std::vector<cv::Point>& points)
{
	const int sz = (int)points.size();
	int distMin = 0;
	int topLeftIdx = -1;
	for (int i = 0; i < sz; i++) {
		const cv::Point& pt = points[i];		// Alias
		const int distFromOrg = std::min(pt.x, pt.y);
		if (topLeftIdx < 0 || distFromOrg < distMin) {
			distMin = distFromOrg;
			topLeftIdx = i;
		}
	}
	return topLeftIdx;
}

/// ���v���ŗׂ̍��W���擾����B
int ClickedPointList::get_clockwise_neighbor(const cv::Point& pt1, const std::vector<cv::Point>& points)
{
	const int sz = (int)points.size();
	cv::Point2d cur_nvec;
	int neigborIdx = -1;
	for (int i = 0; i < sz; i++) {
		const cv::Point& pt2 = points[i];			// Alias
		if (pt2 == pt1) {
			continue;
		}

		// ���㒸�_�̍��Wpt1����ʂ̒��_���Wp2�Ɍ������x�N�g��relVec
		const cv::Point relVec = pt2 - pt1;

		// relVec�ƕ��s�ȒP�ʃx�N�g��evec
		const double relVecLen = get_vec_len(relVec);
		const cv::Point2d evec = cv::Point2d((double)relVec.x / relVecLen, (double)relVec.y / relVecLen);

		// evec��cur_nvec�̓���
		// evec�Acur_nvec�Ƃ��ɐ��K������Ă���̂ŁA�����tan(evec��cur_nvec�̂Ȃ��p)�ł�����B
		const double ipr = evec.x * cur_nvec.x + evec.y * cur_nvec.y;

		if (neigborIdx < 0 || ipr < 0.0) {
			// (p2��p1�ƈقȂ�ŏ��̍��W�A�܂���p2��p1���猩��cur_nvec���K�肷�镽�ʂ̔����v��葤)

			// cur_nvec�X�V
			// �x�N�g��relVec�̍���n�̍��W�n�ɂ����鎞�v���90�x��](����(����)p1-p2�̖@���x�N�g��)
			cur_nvec = cv::Point2d(-evec.y, evec.y);

			// p2��index���L��
			neigborIdx = i;
		}
	}
	return neigborIdx;
}

/// �ł����ォ�玞�v���̏��̃��X�g���擾����B
std::vector<cv::Point> ClickedPointList::get_clockwize_list(std::vector<cv::Point>& points)
{
	std::vector<cv::Point> cwPtList;

	// �ł�����̓_�擾
	{
		const int topLeftIdx = get_most_topleft(points);
		if (topLeftIdx < 0) {
			throw std::logic_error("*** ERR ***");
		}
		assert(0 <= topLeftIdx && (size_t)topLeftIdx < points.size());
		cwPtList.push_back(points[topLeftIdx]);
		points.erase(std::next(points.begin(), topLeftIdx));
	}

	// ���̑��̓_�������擾
	while (!points.empty()) {
		const cv::Point& pt1 = cwPtList.back();		// Alias
		const int neighborIdx = get_clockwise_neighbor(pt1, points);
		if (neighborIdx < 0) {
			throw std::logic_error("*** ERR ***");
		}
		assert(0 <= neighborIdx && (size_t)neighborIdx < points.size());
		cwPtList.push_back(points[neighborIdx]);
		points.erase(std::next(points.begin(), neighborIdx));
	}

	return cwPtList;
}
