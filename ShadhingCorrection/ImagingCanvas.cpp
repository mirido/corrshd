#include "stdafx.h"
#include "ImagingCanvas.h"

#include "geometryutil.h"
#include "imaging_op.h"

// [CONF] ガイド線の描画色
#define GUIDE_COLOR		cv::Scalar(0, 0, 255)

ImagingCanvas::ImagingCanvas()
{
	/*pass*/
}

/// ソース画像を設定する。
void ImagingCanvas::setSrcImage(cv::Ptr<cv::Mat> pSrcImage)
{
	m_pSrcImage = pSrcImage;
}

/// キャンバスを更新する。
bool ImagingCanvas::setupCanvas(const cv::Rect& srcArea, const cv::Size& dispSize)
{
	m_srcArea = srcArea;
	m_dispSize = dispSize;

	// ソース画像のm_srcArea内のみ切り抜き
	cv::Mat ROISrc = cv::Mat(*m_pSrcImage, m_srcArea);
	cv::Mat resized;
	cv::resize(ROISrc, resized, m_dispSize);

	// BGR画像に変換
	if (!conv_color_to_BGR(resized, m_canvas)) {
		return false;
	}

	// できたキャンバスをPolyLine消去用に記憶
	m_canvasMaster = m_canvas.clone();

	// Dirty areaクリア
	m_dirtyArea = cv::Rect();

	return true;
}

/// マウスクリック位置をソース画像上の座標に変換する。
cv::Point ImagingCanvas::convToSrcPoint(const int dispX, const int dispY)
{
	const int srcX = m_srcArea.x + (dispX * m_srcArea.width) / m_dispSize.width;
	const int srcY = m_srcArea.y + (dispY * m_srcArea.height) / m_dispSize.height;
	return cv::Point(srcX, srcY);
}

/// ソース画像上の座標をマウスクリック位置に変換する。
void ImagingCanvas::convToDispPoint(const cv::Point& srcPt, int& dispX, int& dispY)
{
	dispX = (srcPt.x * m_dispSize.width) / m_srcArea.width - m_srcArea.x;
	dispY = (srcPt.y * m_dispSize.height) / m_srcArea.height - m_srcArea.y;
}

/// キャンバスに多角形を描画する。
void ImagingCanvas::drawPolylines(const std::vector<cv::Point>& vertexes, const int vtxMarkerRadius, const int curIdx)
{
	if (vertexes.empty()) {
		return;
	}

	// キャンバス上の座標に変換
	const auto vtxs = convToCanvasPointInBulk(vertexes);

	// 多角形描画
	int npts[1];
	npts[0] = (int)vtxs.size();
	const cv::Point* ppts[1];
	ppts[0] = &(vtxs[0]);
	if (npts[0] >= 2) {
		cv::polylines(m_canvas, ppts, npts, 1, true, GUIDE_COLOR, 1);
	}

	// 頂点マーカー描画
	int sxMin = vtxs.front().x;
	int syMin = vtxs.front().y;
	int exMax = vtxs.front().x;
	int eyMax = vtxs.front().y;
	for (auto it = vtxs.begin(); it != vtxs.end(); it++) {
		const int sx = it->x - vtxMarkerRadius;
		const int ex = sx + 2 * vtxMarkerRadius;
		const int sy = it->y - vtxMarkerRadius;
		const int ey = sy + 2 * vtxMarkerRadius;

		const int thickness = (it - vtxs.begin() == (ptrdiff_t)curIdx) ? 3 : 1;
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
		cv::Mat ROISrc = cv::Mat(m_canvasMaster, dirtyRect);
		cv::Mat ROIDst = cv::Mat(m_canvas, dirtyRect);
		//cv::rectangle(ROISrc, cv::Point(0, 0), cv::Point(ROISrc.cols - 1, ROISrc.rows - 1), cv::Scalar(255, 0, 0), 5);		// DEBUG
		ROISrc.copyTo(ROIDst);
	}
}

/// キャンバスを90°回転する。(座標系は左上原点前提)
void ImagingCanvas::rotate(const int dir, cv::Point& ofsAfterRot)
{
	if (dir == 0) {
		ofsAfterRot = cv::Point(0, 0);
		return;
	}

	// ソース画像回転
	const auto dirFlag = (dir < 0) ? cv::ROTATE_90_COUNTERCLOCKWISE : cv::ROTATE_90_CLOCKWISE;
	cv::Mat dstImg;
	cv::rotate(*m_pSrcImage, dstImg, dirFlag);
	*m_pSrcImage = dstImg;

	// 回転結果を第1象限に平行移動するオフセットofsAfterRot
	if (dir < 0) {
		// (左上原点で反時計回り)
		// 第4象限から移動するオフセットを設定
		ofsAfterRot = cv::Point(0, m_pSrcImage->rows - 1);
	}
	else {
		// (左上原点で時計回り)
		// 第2象限から移動するオフセットを設定
		ofsAfterRot = cv::Point(m_pSrcImage->cols - 1, 0);
	}
	/*
		(NOTE)
		OpenCVを含め、画像上の座標系は一般に左上原点である。
		このため、原点を共有する領域
			O --- A
			|     |
			C --- B
		を画像とともに90°回転させた場合、
		反時計回りなら回転後のAを原点(0, 0)に移す平行移動、
		時計回りなら回転後のCを原点(0, 0)に移す平行移動
		を回転後にそれぞれ行なう必要がある。
		(さもないと、回転結果の左上が原点にならない。)
		上のオフセットofsAfterRotはこの平行移動を表す。
	*/

	// ソース領域90°回転
	// 画像と同じく回転を経ても第1象限とする。
	m_srcArea = rotate_rect(m_srcArea, dir);
	m_srcArea.x += ofsAfterRot.x;
	m_srcArea.y += ofsAfterRot.y;

	// 表示サイズの縦横入れ替え
	std::swap(m_dispSize.width, m_dispSize.height);

	// キャンバス更新
	setupCanvas(m_srcArea, m_dispSize);
}

/// キャンバスを参照する。
cv::Mat& ImagingCanvas::refCanvas()
{
	return m_canvas;
}

//
//	Private
//

/// ソース画像上の座標をキャンバス上の座標に変換
cv::Point ImagingCanvas::convToCanvasPoint(const cv::Point& point) const
{
	cv::Point dstPt(point);

	dstPt.x -= m_srcArea.x;
	dstPt.y -= m_srcArea.y;

	dstPt.x = (dstPt.x * m_dispSize.width) / m_srcArea.width;
	dstPt.y = (dstPt.y * m_dispSize.height) / m_srcArea.height;

	return dstPt;
}

/// ソース画像上の座標をキャンバス上の座標に一括変換
std::vector<cv::Point> ImagingCanvas::convToCanvasPointInBulk(const std::vector<cv::Point>& points) const
{
	const size_t sz = points.size();

	std::vector<cv::Point> dst(sz);
	for (size_t i = 0; i < sz; i++) {
		dst[i] = convToCanvasPoint(points[i]);
	}

	return dst;
}
