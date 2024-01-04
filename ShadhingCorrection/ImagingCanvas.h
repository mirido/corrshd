#pragma once

class ImagingCanvas
{
	cv::Ptr<cv::Mat> m_pSrcImage;
	cv::Mat m_canvas;
	cv::Rect m_dirtyArea;

public:
	ImagingCanvas();

	/// ソース画像を設定する。
	bool setSrcImage(cv::Ptr<cv::Mat> pSrcImage);

	/// キャンバスに多角形を描画する。
	void drawPolylines(const std::vector<cv::Point>& vertexes, const int vtxMarkerRadius, const double magToDisp);

	/// キャンバスへの描画を消去する。
	void cleanup();

	/// キャンバスを90°回転する。(座標系は左手系前提)
	void rotate(const int dir, cv::Point& ofsAfterRot);

	/// ソース画像を参照する。
	cv::Ptr<cv::Mat> getSrcImagePtr();

	/// キャンバスを参照する。
	cv::Mat& refCanvas();

};
