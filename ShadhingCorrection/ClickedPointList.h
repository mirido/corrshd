#pragma once
struct ClickedPointList
{
	std::vector<cv::Point2f> m_points;
	int m_curIdx;

	/*
		(NOTE)
		表示範囲の幅か高さの画素数が偶数の画像についての回転について
		回転後の座標を正確に表すため、m_pointsの要素はcv::Pointでなく
		cv::Point2fである必要がある。
	*/

	ClickedPointList();

	void clear();

	/// (x, y)に最も近い既存要素を探す。
	int selectFromExisting(const int x, const int y, int& nearestDist) const;

	/// 座標を選択または追加する。
	void selectOrAdd(const int x, const int y);

	/// 最も左上から時計回りの順のリストを取得する。
	std::vector<cv::Point2f> getClockwizeLlist() const;

	/// 現在選択中の座標を取得する。
	cv::Point2f getCurPoint() const;

	/// 座標列の外接矩形を取得する。
	bool getOutRect(cv::Rect2f& rect) const;

	/// キャンバスを90°回転する。(座標系は左手系前提)
	void rotate(const cv::Point2f& centerPt, const int dir);

private:
	/// 最も左上の座標を取得する。
	static int get_most_topleft(const std::vector<cv::Point2f>& points);

	/// 時計回りで隣の座標を取得する。
	static int get_clockwise_neighbor(const cv::Point2f& pt1, const std::vector<cv::Point2f>& points);

	/// 最も左上から時計回りの順のリストを取得する。
	static std::vector<cv::Point2f> get_clockwize_list(std::vector<cv::Point2f>& points);

};
