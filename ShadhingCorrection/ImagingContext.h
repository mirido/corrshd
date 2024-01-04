#pragma once

struct ImagingContext
{
private:
	ImagingCanvas m_imagingCanvas;
	ClickedPointList m_clickedPointList;

	cv::Rect m_srcArea;
	int m_dispWidth;

public:
	ImagingContext();

	/// ソース画像設定
	void setSrcImage(cv::Ptr<cv::Mat> pSrcImage);

	/// 表示仕様設定
	void setDispGeometry(const cv::Rect& srcArea, const int dispWidth);

	/// 座標初期化
	void clearPointList();

	/// 座標追加
	void selectOrAdd(const int x, const int y);

	/// キャンバス再描画
	void refreshCanvas();

	/// キャンバスとソース画像回転
	void rotate(const int dir);

	/// 歪み補正
	bool correctDistortion(const double relWidth, const double relHeight, const int outputWidth);

	/// 切り抜き
	bool cutOff();

	/// シェーディング補正
	bool shadingCorrection();

	/// ヒストグラム均等化
	bool equalizeHist();

	/// キャンバスを参照する。
	cv::Mat& refCanvas();

	/// Current pointを取得する。
	bool getCurPt(cv::Point& pt) const;

};
