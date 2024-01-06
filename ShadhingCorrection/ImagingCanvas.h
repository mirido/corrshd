#pragma once

class ImagingCanvas
{
	cv::Ptr<cv::Mat> m_pSrcImage;
	cv::Mat m_canvas;
	cv::Mat m_canvasMaster;
	cv::Rect m_srcArea;
	cv::Size m_dispSize;

	cv::Rect m_dirtyArea;

public:
	ImagingCanvas();

	/// ソース画像を設定する。
	void setSrcImage(cv::Ptr<cv::Mat> pSrcImage);
	
	/// キャンバスを更新する。
	bool setupCanvas(const cv::Rect& srcArea, const cv::Size& dispSize);

	/// マウスクリック位置をソース画像上の座標に変換する。
	cv::Point convToSrcPoint(const int dispX, const int dispY);

	/// ソース画像上の座標をマウスクリック位置に変換する。
	void convToDispPoint(const cv::Point& srcPt, int& dispX, int& dispY);

	/// キャンバスに多角形を描画する。
	void drawPolylines(const std::vector<cv::Point>& vertexes, const int vtxMarkerRadius, const int curIdx);

	/// キャンバスへの描画を消去する。
	void cleanup();

	/// キャンバスを90°回転する。(座標系は左上原点前提)
	void rotate(const int dir, cv::Point& ofsAfterRot);

	/// キャンバスを参照する。
	cv::Mat& refCanvas();

private:
	/// ソース画像上の座標をキャンバス上の座標に変換
	cv::Point convToCanvasPoint(const cv::Point& point) const;

	/// ソース画像上の座標をキャンバス上の座標に一括変換
	std::vector<cv::Point> convToCanvasPointInBulk(const std::vector<cv::Point>& points) const;

};
