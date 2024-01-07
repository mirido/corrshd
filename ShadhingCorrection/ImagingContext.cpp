#include "stdafx.h"
#include "ImagingCanvas.h"
#include "ClickedPointList.h"
#include "ImagingContext.h"

#include "numericutil.h"
#include "imaging_op.h"
#include "geometryutil.h"

// [DBGSW] 中間画像保存先
#define DBG_IMG_DIR		"C:\\usr2\\debug\\"

// [CONF] クリック位置の距離の閾値
// 既存ポイントとのマンハッタン距離が以下の値以下なら既存ポイントの選択とみなす。
#define NEAR_DISTANCE_MAX			16

ImagingContext::ImagingContext()
{
	/*pass*/
}

/// ソース画像設定
void ImagingContext::setSrcImage(cv::Ptr<cv::Mat> pSrcImage)
{
	m_imagingCanvas.setSrcImage(pSrcImage);
	m_pSrcImage = pSrcImage;
}

/// ソース画像参照
cv::Ptr<cv::Mat> ImagingContext::refSrcImage()
{
	return m_pSrcImage;
}

/// キャンバス設定
bool ImagingContext::setupCanvas(const cv::Rect& srcArea, const cv::Size& dispSize)
{
	return m_imagingCanvas.setupCanvas(srcArea, dispSize);
}

/// キャンバス参照
cv::Mat& ImagingContext::refCanvas()
{
	return m_imagingCanvas.refCanvas();
}

/// 座標初期化
void ImagingContext::clearPointList()
{
	m_clickedPointList.clear();
}

/// 既存座標列が空か否かを返す。
bool ImagingContext::isPointListEmpty() const
{
	return m_clickedPointList.empty();
}

/// 既存座標の個数を取得する。
int ImagingContext::pointListSize() const
{
	return m_clickedPointList.size();
}

/// 既存座標列を取得する。
int ImagingContext::getPointList(std::vector<cv::Point>& points) const
{
	return m_clickedPointList.getPointList(points);
}

/// 既存座標選択
bool ImagingContext::selectExistingPointIF(const int dispX, const int dispY)
{
	// 座標(dispX, dispY)をソース画像上の座標srcPtに変換
	const cv::Point srcPt = m_imagingCanvas.convToSrcPoint(dispX, dispY);

	// srcPtに最も近い頂点を検索
	int nearestDist;
	cv::Point foundPt;
	const int selIdx = m_clickedPointList.selectFromExisting(srcPt, nearestDist, foundPt);
	if (selIdx < 0) {
		return false;
	}

	// 表示画像上でNEAR_DISTANCE_MAX画素以内か判定
	int dispPtX, dispPtY;
	m_imagingCanvas.convToDispPoint(foundPt, dispPtX, dispPtY);
	if (!(std::abs(dispX - dispPtX) <= NEAR_DISTANCE_MAX && std::abs(dispY - dispPtY) <= NEAR_DISTANCE_MAX)) {
		return false;
	}

	m_clickedPointList.setCurIdx(selIdx);
	return true;
}

/// 座標の追加または移動
void ImagingContext::addOrMovePoint(const int dispX, const int dispY)
{
	const cv::Point srcPt = m_imagingCanvas.convToSrcPoint(dispX, dispY);
	m_clickedPointList.addOrMovePoint(srcPt);
}

/// Current point取得
bool ImagingContext::getCurPoint(cv::Point& curPt) const
{
	return m_clickedPointList.getCurPoint(curPt);
}

/// Current point移動
void ImagingContext::moveCurPoint(const int dx, const int dy)
{
	m_clickedPointList.moveCurPoint(dx, dy);
}

/// Current point切り替え
void ImagingContext::changeCurrentPointToNext()
{
	m_clickedPointList.changeCurrentPointToNext();
}

/// キャンバス再描画
void ImagingContext::refreshCanvas()
{
	// 描画済みのガイド線消去
	m_imagingCanvas.cleanup();

	// 描画順の頂点リストvertexes取得
	const std::vector<cv::Point> vertexes = m_clickedPointList.getClockwizeLlist();

	// vertexesの中からcurrent point検索
	int curIdx = -1;
	cv::Point curPt;
	if (m_clickedPointList.getCurPoint(curPt)) {
		const int sz = (int)vertexes.size();
		for (int i = 0; i < sz; i++) {
			if (vertexes[i] == curPt) {
				curIdx = i;
				break;
			}
		}
	}

	// 描画
	m_imagingCanvas.drawPolylines(vertexes, NEAR_DISTANCE_MAX, curIdx);
}

/// キャンバスとソース画像回転
void ImagingContext::rotate(const int dir)
{
	// ソース画像90°回転
	cv::Point ofsAfterRot;
	m_imagingCanvas.rotate(dir, ofsAfterRot);

	// 既存座標リスト内容90°回転
	m_clickedPointList.rotate(dir, ofsAfterRot);
}

/// 歪み補正
bool ImagingContext::correctDistortion(const cv::Size& dstSz, cv::Mat& dstImg)
{
	const int nptsExp = 4;

	// グレースケール画像に変換
	cv::Mat grayImage;
	if (!conv_color_to_gray(*m_pSrcImage, grayImage)) {
		return false;
	}

	// 既存座標(歪んだROIの4頂点)を時計回りの順でリスト化
	const std::vector<cv::Point> srcROICorners = m_clickedPointList.getClockwizeLlist();
	if (srcROICorners.size() != nptsExp) {
		return false;
	}

	// cv::Point2fのリストに変換
	cv::Point2f srcROICorners2f[nptsExp];
	for (int i = 0; i < nptsExp; i++) {
		srcROICorners2f[i] = cv::Point2f((float)srcROICorners[i].x, (float)srcROICorners[i].y);
	}

	// 変換実行
	warp_image(grayImage, dstImg, srcROICorners2f, nptsExp, dstSz);

	return true;
}

/// シェーディング補正
bool ImagingContext::doShadingCorrection(const cv::Size& dstSz, cv::Mat& dstImg)
{
	cv::Mat tmp;

	// パースペクティブ補正
	cv::Mat gray1;
	if (!correctDistortion(dstSz, gray1)) {
		cout << __FUNCTION__ << "correctDistortion() failed." << endl;
		return false;
	}
	assert(gray1.channels() == 1);	// 結果はグレースケール画像
#ifndef NDEBUG
	cv::imwrite(DBG_IMG_DIR "01_counter_warped_image.jpg", gray1);
#ifdef DBG_IMG_DIR
	cv::imwrite(DBG_IMG_DIR "01_counter_warped_image.jpg", gray1);
#endif
#endif

	// 明るさのむらを均一化(ブラックハット演算、gray2)
	// クロージング結果 - 原画像、という演算なので、均一化とともに背景と線の輝度が反転する。
	// 以下の行末コメントは、dstSz.width == 800 の下でカーネルサイズを変えて実験した結果。
	//cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));		// 線がかすれる
	//cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
	//cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(10, 10));
	//cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(20, 20));		// 良好
	//cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(50, 50));		// 遅い
	const int knsz = (int)std::round(std::max(dstSz.width, dstSz.height) * 0.025);
	const cv::Size kernelSz = cv::Size(knsz, knsz);
	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, kernelSz);
	cv::Mat gray2;
	cv::morphologyEx(gray1, gray2, cv::MORPH_BLACKHAT, kernel);
#ifndef NDEBUG
	cv::imshow("02_image_after_black_hat", gray2);
#ifdef DBG_IMG_DIR
	cv::imwrite(DBG_IMG_DIR "02_image_after_black_hat.jpg", gray2);
#endif
#endif

	// 以下、gray2を均一化画像と呼ぶ。

	// 均一化画像gray2平滑化(gray1)
	cv::blur(gray2, gray1, cv::Size(3, 3));

	// 平滑化結果gray1に対し、大津の方法で閾値th1を算出
	const double th1 = cv::threshold(gray1, tmp, 0, 255, cv::THRESH_OTSU);
	cout << "th1=" << th1 << endl;
	tmp.release();		// 2値化結果は使わない

#ifndef NDEBUG
	// get_unmasked_data()のテスト
	{
		cv::Mat nonmask = cv::Mat::ones(gray1.rows, gray1.cols, CV_8UC1);
		const std::vector<uchar> dbg_data = get_unmasked_data(gray1, nonmask);
		const int dbg_th1 = discriminant_analysis_by_otsu(dbg_data);
		cout << "dbg_th1=" << dbg_th1 << endl;
		if (dbg_th1 != (int)std::round(th1)) {
			throw std::logic_error("*** ERR ***");
		}
	}
#endif

	// マスク作成
	// 平滑化画像gray1の輝度th1以下を黒(0)、超過を白(255)にする(mask)
	cv::Mat mask;
	cv::threshold(gray1, mask, th1, 255.0, cv::THRESH_BINARY);
#ifndef NDEBUG
	cv::imshow("03_mask", mask);
#ifdef DBG_IMG_DIR
	cv::imwrite(DBG_IMG_DIR "03_mask.jpg", mask);
#endif
#endif

	// 均一化画像gray2のマスクされない画素データを抽出(data)
	std::vector<uchar> data = get_unmasked_data(gray1, mask);
	if (data.size() < 2) {
		return false;
	}

	// dataに対し大津の方法で判別分析を実施
	const int th2 = discriminant_analysis_by_otsu(data);
	cout << "th2=" << th2 << endl;

	// dataの最小値を取得
	const int minv = *std::min_element(data.begin(), data.end());
	cout << "minv=" << minv << endl;

	const int th3 = minv + (int)((double)(th2 - minv) * 0.2);
	cout << "th3=" << th3 << endl;

	// 均一化画像gray2の閾値th3以下を黒(0)、超える画素を原画像のままとする(gray2)
	cv::threshold(gray2, gray2, th3, 255.0, cv::THRESH_TOZERO);
#ifndef NDEBUG
	cv::imshow("04_threshold_by_th3", gray2);
#ifdef DBG_IMG_DIR
	cv::imwrite(DBG_IMG_DIR "04_threshold_by_th3.jpg", gray2);
#endif
#endif

	// gray2を白黒反転(dstImg)
	cv::bitwise_not(gray2, dstImg);

	return true;
}
