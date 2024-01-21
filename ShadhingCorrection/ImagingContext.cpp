#include "stdafx.h"
#include "ClickedPointList.h"
#include "ImagingCanvas.h"
#include "IImgFunc.h"
#include "ImagingContext.h"

#include "ImgFuncBase.h"
#include "ImgFunc_shdc01.h"
#include "ImgFunc_shdc02.h"

#include "imaging_op.h"

// [CONF] クリック位置の距離の閾値
// 既存ポイントとのマンハッタン距離が以下の値以下なら既存ポイントの選択とみなす。
#define NEAR_DISTANCE_MAX			16

ImagingContext::ImagingContext()
{
	// Select algorithm.
	// TODO: Make it variable by command line arguments.
	//m_pImgFunc = std::unique_ptr<IImgFunc>(new ImgFunc_shdc01);
	m_pImgFunc = std::unique_ptr<IImgFunc>(new ImgFunc_shdc02);
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

/// 最も左上から時計回りの順のリストを取得する。
std::vector<cv::Point> ImagingContext::getClockwiseList() const
{
	return m_clickedPointList.getClockwiseLlist();
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
	const std::vector<cv::Point> vertexes = m_clickedPointList.getClockwiseLlist();

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
	const std::vector<cv::Point> srcROICorners = m_clickedPointList.getClockwiseLlist();
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

#ifndef NDEBUG
	cout << std::setbase(10);
#endif

	m_pImgFunc->resetImgDumpCnt();

	// Perspective correction
	cv::Mat gray1;
	if (!correctDistortion(dstSz, gray1)) {
		cout << __FUNCTION__ << "correctDistortion() failed." << endl;
		return false;
	}
	assert(gray1.channels() == 1);	// 結果はグレースケール画像
	m_pImgFunc->dumpImg(gray1, "counter_warped_image", DBG_IMG_DIR);

	// Image processing after perspective correction
	return m_pImgFunc->run(gray1, dstImg);
}
