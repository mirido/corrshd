#include "stdafx.h"
#include "ImagingCanvas.h"
#include "ClickedPointList.h"
#include "ImagingContext.h"

#include "imaging_op.h"
#include "geometryutil.h"

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

/// キャンバス更新
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

/// Current point変更
bool ImagingContext::selectExistingPointIF(const int dispX, const int dispY)
{
	const cv::Point srcPt = m_imagingCanvas.convToSrcPoint(dispX, dispY);

	int nearestDist;
	const int selIdx = m_clickedPointList.selectFromExisting(srcPt, nearestDist);
	if (selIdx < 0 || nearestDist > NEAR_DISTANCE_MAX) {
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
bool ImagingContext::correctDistortion(const double relWidth, const double relHeight, const int outputWidth)
{
	const int nptsExp = 4;

	const std::vector<cv::Point> srcPts = m_clickedPointList.getClockwizeLlist();
	if (srcPts.size() != nptsExp) {
		return false;
	}

	cv::Point2f srcPts2f[nptsExp];
	for (int i = 0; i < nptsExp; i++) {
		srcPts2f[i] = cv::Point2f((float)srcPts[i].x, (float)srcPts[i].y);
	}

	cv::Mat dstImg;
	warp_image(*m_pSrcImage, dstImg, srcPts2f, nptsExp, relWidth, relHeight, outputWidth);
	*m_pSrcImage = dstImg;
	return true;
}

/// 切り抜き
bool ImagingContext::cutOff()
{
	return false;
}

/// シェーディング補正
bool ImagingContext::shadingCorrection()
{
	return false;
}

/// ヒストグラム均等化
bool ImagingContext::equalizeHist()
{
	return false;
}
