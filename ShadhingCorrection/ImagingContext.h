#pragma once

class ImagingContext
{
	cv::Ptr<cv::Mat> m_pSrcImage;
	ImagingCanvas m_imagingCanvas;
	ClickedPointList m_clickedPointList;

	// Image processing algorithm after perspective correction
	std::unique_ptr<IImgFunc> m_pImgFunc;

public:
	ImagingContext();

	/// ソース画像設定
	void setSrcImage(cv::Ptr<cv::Mat> pSrcImage);

	/// ソース画像参照
	cv::Ptr<cv::Mat> refSrcImage();

	/// キャンバス設定
	bool setupCanvas(const cv::Rect& srcArea, const cv::Size& dispSize);

	/// キャンバス参照
	cv::Mat& refCanvas();

	/// 座標初期化
	void clearPointList();

	/// 既存座標列が空か否かを返す。
	bool isPointListEmpty() const;

	/// 既存座標の個数を取得する。
	int pointListSize() const;

	/// 既存座標列を取得する。
	int getPointList(std::vector<cv::Point>& points) const;

	/// 最も左上から時計回りの順のリストを取得する。
	std::vector<cv::Point> getClockwiseList() const;

	/// 既存座標選択
	bool selectExistingPointIF(const int dispX, const int dispY);

	/// 座標の追加または移動
	void addOrMovePoint(const int dispX, const int dispY);

	/// Current point取得
	bool getCurPoint(cv::Point& curPt) const;

	/// Current point移動
	void moveCurPoint(const int dx, const int dy);

	/// Current point切り替え
	void changeCurrentPointToNext();

	/// キャンバス再描画
	void refreshCanvas();

	/// キャンバスとソース画像回転
	void rotate(const int dir);

	/// 歪み補正
	bool correctDistortion(const cv::Size& dstSz, cv::Mat& dstImg);

	/// シェーディング補正
	bool doShadingCorrection(const cv::Size& dstSz, cv::Mat& dstImg);

};
