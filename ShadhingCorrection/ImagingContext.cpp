#include "stdafx.h"
#include "ImagingCanvas.h"
#include "ClickedPointList.h"
#include "ImagingContext.h"

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
	const int thickness = std::min(1, m_srcArea.width / m_dispWidth);

	// 描画済みのガイド線消去
	m_imagingCanvas.cleanup();

	// ガイド線描画
	const std::vector<cv::Point> vertexes = m_clickedPointList.getClockwizeLlist();
	m_imagingCanvas.drawPolylines(vertexes, NEAR_DISTANCE_MAX, thickness);
}

/// キャンバスとソース画像回転
void ImagingContext::rotate(const int dir)
{
	m_imagingCanvas.rotate(dir);
	m_clickedPointList.rotate(dir);
}

/// 歪み補正
bool ImagingContext::correctDistortion(const double relWidth, const double relHeight, const int outputWidth)
{
	const std::vector<cv::Point> vertexes = m_clickedPointList.getClockwizeLlist();
	if (vertexes.size() != 4) {
		return false;
	}

	// 変換前の4点
	cv::Point2f srcPts[4];
	for (int i = 0; i < 4; i++) {
		srcPts[i].x = (float)vertexes[i].x;
		srcPts[i].y = (float)vertexes[i].y;
	}

	// 変換後の4点
	const int nOutWidth = outputWidth;
	const int nOutHeight = (int)std::round(((double)outputWidth * relHeight) / relWidth);
	const float left = (float)nOutWidth;
	const float btm = (float)nOutHeight;
	cv::Point2f dstPts[4];
	dstPts[0] = cv::Point2f(0.0F, 0.0F);
	dstPts[1] = cv::Point2f(left, 0.0F);
	dstPts[2] = cv::Point2f(left, btm);
	dstPts[3] = cv::Point2f(0.0f, btm);

	// 変換行列取得
	const cv::Mat M = cv::getPerspectiveTransform(srcPts, dstPts);

	// 変換実施
	cv::Ptr<cv::Mat> pTransformed;
	cv::warpPerspective(*(m_imagingCanvas.getSrcImagePtr()), *pTransformed, M, cv::Size(nOutWidth, nOutHeight));

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
