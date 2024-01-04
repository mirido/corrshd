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
	: m_dispWidth(0)
{
	/*pass*/
}

/// キャンバス設定
void ImagingContext::setSrcImage(cv::Ptr<cv::Mat> pSrcImage)
{
	m_imagingCanvas.setSrcImage(pSrcImage);
	m_srcArea = cv::Rect(0, 0, pSrcImage->cols, pSrcImage->rows);
	m_dispWidth = pSrcImage->cols;
}

/// 表示仕様設定
void ImagingContext::setDispGeometry(const cv::Rect& srcArea, const int dispWidth)
{
	m_srcArea = srcArea;
	m_dispWidth = dispWidth;
}

/// 座標初期化
void ImagingContext::clearPointList()
{
	m_clickedPointList.clear();
}

/// 座標追加
void ImagingContext::selectOrAdd(const int x, const int y)
{
	const int srcWidth = m_srcArea.width;
	const int srcX = m_srcArea.x + (x * srcWidth) / m_dispWidth;
	const int srcY = m_srcArea.y + (y * srcWidth) / m_dispWidth;

	m_clickedPointList.selectOrAdd(srcX, srcY);
}

/// キャンバス更新
void ImagingContext::refreshCanvas()
{
	const double magToDisp = (double)m_srcArea.width / (double)m_dispWidth;

	// 描画済みのガイド線消去
	m_imagingCanvas.cleanup();

	// ガイド線描画
	const std::vector<cv::Point> vertexes = m_clickedPointList.getClockwizeLlist();
	m_imagingCanvas.drawPolylines(vertexes, NEAR_DISTANCE_MAX, magToDisp);
}

/// キャンバスとソース画像回転
void ImagingContext::rotate(const int dir)
{
	// ソース画像90°回転
	// 画像は回転を経ても第1象限にあり続ける。
	cv::Point ofsAfterRot;
	m_imagingCanvas.rotate(dir, ofsAfterRot);

	// ソース領域90°回転
	// 画像と同じく回転を経ても第1象限とする。
	rotate_rect(m_srcArea, dir);
	m_srcArea.x += ofsAfterRot.x;
	m_srcArea.y += ofsAfterRot.y;

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
	warp_image(*(m_imagingCanvas.getSrcImagePtr()), dstImg, srcPts2f, nptsExp, relWidth, relHeight, outputWidth);
	*(m_imagingCanvas.getSrcImagePtr()) = dstImg;
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

/// キャンバスを参照する。
cv::Mat& ImagingContext::refCanvas()
{
	return m_imagingCanvas.refCanvas();
}

/// Current pointを取得する。
bool ImagingContext::getCurPt(cv::Point& pt) const
{
	const int curIdx = m_clickedPointList.m_curIdx;

	if (curIdx < 0) {
		pt = cv::Point();
		return false;
	}
	else {
		assert(0 <= curIdx && (size_t)curIdx < m_clickedPointList.m_points.size());
		pt = m_clickedPointList.m_points[curIdx];
		return true;
	}
}
