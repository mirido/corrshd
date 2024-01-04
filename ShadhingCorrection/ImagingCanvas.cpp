#include "stdafx.h"
#include "ImagingCanvas.h"

#include "geometryutil.h"

// [CONF] ガイド線の描画色
#define GUIDE_COLOR		cv::Scalar(0, 0, 255)

ImagingCanvas::ImagingCanvas()
{
	/*pass*/
}

/// ソース画像を設定する。
bool ImagingCanvas::setSrcImage(cv::Ptr<cv::Mat> pSrcImage)
{
	// パラメータ記憶
	m_pSrcImage = pSrcImage;

	switch (m_pSrcImage->channels()) {
	case 3:
		m_pSrcImage->copyTo(m_canvas);
		break;
	case 1:
		cv::cvtColor(*m_pSrcImage, m_canvas, cv::COLOR_GRAY2BGR);
		break;
	default:
		return false;
	}

	// Dirty areaクリア
	m_dirtyArea = cv::Rect();

	return true;
}

/// キャンバスに多角形を描画する。
void ImagingCanvas::drawPolylines(const std::vector<cv::Point>& vertexes, const int vtxMarkerRadius, const int thickness)
{
	if (vertexes.empty()) {
		return;
	}

	// 多角形表示
	int npts[1];
	npts[0] = (int)vertexes.size();
	const cv::Point* ppts[1];
	ppts[0] = &(vertexes[0]);
	cv::polylines(m_canvas, ppts, npts, 1, true, GUIDE_COLOR, thickness);

	// 頂点マーカー描画
	int sxMin = vertexes.front().x;
	int syMin = vertexes.front().y;
	int exMax = vertexes.front().x;
	int eyMax = vertexes.front().y;
	for (auto it = vertexes.begin(); it != vertexes.end(); it++) {
		const int sx = it->x - vtxMarkerRadius;
		const int ex = sx + 2 * vtxMarkerRadius;
		const int sy = it->y - vtxMarkerRadius;
		const int ey = sy + 2 * vtxMarkerRadius;

		cv::rectangle(m_canvas, cv::Point(sx, sy), cv::Point(ex, ey), GUIDE_COLOR, thickness);

		sxMin = std::min(sxMin, sx - (vtxMarkerRadius + thickness));
		syMin = std::min(syMin, sy - (vtxMarkerRadius + thickness));
		exMax = std::max(exMax, ex + (vtxMarkerRadius + thickness));
		eyMax = std::max(eyMax, ey + (vtxMarkerRadius + thickness));
	}

	// Dirty area記憶
	sxMin = std::max(sxMin, 0);
	syMin = std::max(syMin, 0);
	exMax = std::min(exMax, m_canvas.cols);
	eyMax = std::min(eyMax, m_canvas.rows);
	const cv::Rect rect = cv::Rect(sxMin, syMin, exMax - sxMin, eyMax - syMin);
	m_dirtyArea = clip_rect_into_image(rect, m_canvas.cols, m_canvas.rows);
}

/// キャンバスへの描画を消去する。
void ImagingCanvas::cleanup()
{
	const cv::Rect dirtyRect = m_dirtyArea;

	if (!is_empty_rect(dirtyRect)) {
		cv::Mat ROISrc = cv::Mat(*m_pSrcImage, dirtyRect);
		cv::Mat ROIDst = cv::Mat(m_canvas, dirtyRect);
		//scv::rectangle(ROISrc, cv::Point(0, 0), cv::Point(ROISrc.cols - 1, ROISrc.rows - 1), cv::Scalar(255, 0, 0), 5);		// DEBUG
		ROISrc.copyTo(ROIDst);
	}
}

/// キャンバスを90°回転する。(座標系は左手系前提)
void ImagingCanvas::rotate(const int dir)
{
	if (dir == 0) {
		return;
	}

	// ソース画像回転
	const auto dirFlag = (dir < 0) ? cv::ROTATE_90_COUNTERCLOCKWISE : cv::ROTATE_90_CLOCKWISE;
	cv::rotate(*m_pSrcImage, *m_pSrcImage, dirFlag);

	// キャンバス再設定
	setSrcImage(m_pSrcImage);
}

/// ソース画像を参照する。
cv::Ptr<cv::Mat> ImagingCanvas::getSrcImagePtr()
{
	return m_pSrcImage;
}

/// キャンバスを参照する。
cv::Mat& ImagingCanvas::refCanvas()
{
	return m_canvas;
}
