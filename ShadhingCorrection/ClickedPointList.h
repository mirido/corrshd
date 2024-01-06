#pragma once
struct ClickedPointList
{
	std::vector<cv::Point> m_points;
	int m_curIdx;

	ClickedPointList();

	void clear();

	/// 指定座標に最も近い既存座標を選択する。
	int selectFromExisting(const cv::Point& srcPt, int& nearestDist, cv::Point& foundPt) const;

	/// 座標を追加または移動する。
	void addOrMovePoint(const cv::Point& srcPt);

	/// 最も左上から時計回りの順のリストを取得する。
	std::vector<cv::Point> getClockwizeLlist() const;

	/// Cureent indexを設定する。
	void setCurIdx(const int idx);

	/// Current pointを移動する。
	bool moveCurPoint(const int dx, const int dy);

	/// Current pointを巡回的に切り替える。
	void changeCurrentPointToNext();

	/// Current pointを取得する。
	bool getCurPoint(cv::Point& curPt) const;

	/// 座標列の外接矩形を取得する。
	bool getOutRect(cv::Rect& rect) const;

	/// キャンバスを90°回転する。(座標系は左上原点前提)
	void rotate(const int dir, const cv::Point& ofsAfterRot);

private:
	/// 最も左上の座標を取得する。
	static int get_most_topleft(const std::vector<cv::Point>& points);

	/// 時計回りで隣の座標を取得する。
	static int get_clockwise_neighbor(const cv::Point& pt1, const std::vector<cv::Point>& points);

	/// 最も左上から時計回りの順のリストを取得する。
	static std::vector<cv::Point> get_clockwize_list(std::vector<cv::Point>& points);

};
