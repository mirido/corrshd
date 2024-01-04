#include "stdafx.h"
#include "ClickedPointList.h"

#include "geometryutil.h"

// [CONF] 選択する点の数
#define NPOINTS_MAX					4

namespace
{
	/// ベクトルの長さを取得する。
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

/// (x, y)に最も近い既存要素を探す。
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

	// 整合性チェック
	// 最も近い既存要素が見つからないのはm_pointsが空のときのみのはず。
	if (nearestIdx < 0) {
		if (!m_points.empty()) {
			throw std::logic_error("*** ERR ***");
		}
	}

	return nearestIdx;
}

/// 座標を選択または追加する。
void ClickedPointList::selectOrAdd(const int x, const int y)
{
	int nearestDist;
	const int foundIdx = selectFromExisting(x, y, nearestDist);
	if (foundIdx < 0 || (nearestDist > 0 && m_points.size() < NPOINTS_MAX)) {
		// (既存要素が見つからなかったか、
		//  既存のどれとも異なる座標が選択され、かつ座標の数が最大数に達していない)
		// 追加
		m_points.push_back(cv::Point(x, y));
		m_curIdx = (int)(m_points.size() - 1);
	}
	else {
		// 最も近い既存要素を選択
		assert(0 <= foundIdx && (size_t)foundIdx < m_points.size());
		m_points[foundIdx] = cv::Point(x, y);
		m_curIdx = foundIdx;
	}
}

/// 最も左上から時計回りの順のリストを取得する。
std::vector<cv::Point> ClickedPointList::getClockwizeLlist() const
{
	std::vector<cv::Point> points(m_points);
	return get_clockwize_list(points);
}

/// 現在選択中の座標を取得する。
cv::Point ClickedPointList::getCurPoint() const
{
	if (m_curIdx < 0) {
		// (既存座標無し)
		throw std::logic_error("*** ERR ***");
	}
	assert(0 <= m_curIdx && m_curIdx < (size_t)m_points.size());
	return m_points[m_curIdx];
}

/// 座標列の外接矩形を取得する。
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

/// キャンバスを90°回転する。(座標系は左手系前提)
void ClickedPointList::rotate(const int dir)
{
	for (auto it = m_points.begin(); it != m_points.end(); it++) {
		rotate_point(*it, dir);
	}
}

//
//	Private
//

/// 最も左上の座標を取得する。
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

/// 時計回りで隣の座標を取得する。
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

		// 左上頂点の座標pt1から別の頂点座標p2に向かうベクトルrelVec
		const cv::Point relVec = pt2 - pt1;

		// relVecと平行な単位ベクトルevec
		const double relVecLen = get_vec_len(relVec);
		const cv::Point2d evec = cv::Point2d((double)relVec.x / relVecLen, (double)relVec.y / relVecLen);

		// evecとcur_nvecの内積
		// evec、cur_nvecともに正規化されているので、これはtan(evecとcur_nvecのなす角)でもある。
		const double ipr = evec.x * cur_nvec.x + evec.y * cur_nvec.y;

		if (neigborIdx < 0 || ipr < 0.0) {
			// (p2はp1と異なる最初の座標、またはp2はp1から見てcur_nvecが規定する平面の反時計回り側)

			// cur_nvec更新
			// ベクトルrelVecの左手系の座標系における時計回り90度回転(平面(直線)p1-p2の法線ベクトル)
			cur_nvec = cv::Point2d(-evec.y, evec.y);

			// p2のindexを記憶
			neigborIdx = i;
		}
	}
	return neigborIdx;
}

/// 最も左上から時計回りの順のリストを取得する。
std::vector<cv::Point> ClickedPointList::get_clockwize_list(std::vector<cv::Point>& points)
{
	std::vector<cv::Point> cwPtList;

	// 最も左上の点取得
	{
		const int topLeftIdx = get_most_topleft(points);
		if (topLeftIdx < 0) {
			throw std::logic_error("*** ERR ***");
		}
		assert(0 <= topLeftIdx && (size_t)topLeftIdx < points.size());
		cwPtList.push_back(points[topLeftIdx]);
		points.erase(std::next(points.begin(), topLeftIdx));
	}

	// その他の点を順次取得
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
