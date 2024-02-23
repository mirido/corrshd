#include "stdafx.h"
#include "ClickedPointList.h"
#include "../libimaging/imaging_op.h"
#include "ImagingCanvas.h"
#include "IImgFunc.h"
#include "ImgFuncBase.h"
#include "ImagingContext.h"

#include "ImgFunc_shdc01.h"
#include "ImgFunc_shdc02.h"
#include "ImgFunc_whitening01.h"
#include "ImgFunc_whitening02.h"
#include "ImgFunc_uniform.h"
#include "ImgFunc_shdc03.h"
#include "ImgFunc_shdc04.h"

// [CONF] クリック位置の距離の閾値
// 既存ポイントとのマンハッタン距離が以下の値以下なら既存ポイントの選択とみなす。
#define NEAR_DISTANCE_MAX			16

namespace
{
	void append_imgfunc(
		IImgFunc* const pImgFunc,
		std::map<std::string, std::shared_ptr<IImgFunc> >& dic,
		std::vector<std::string>& names
	)
	{
		std::string name = pImgFunc->getName();
		auto found_it = dic.find(name);
		if (!(found_it == dic.end())) {
			// (Duplicate names found.)
			throw std::logic_error("*** ERR ***");
		}
		dic[name] = std::shared_ptr<IImgFunc>(pImgFunc);
		names.push_back(name);
	}

}	// namespace

ImagingContext::ImagingContext()
	: m_avoidfg(m_param)
{
	append_imgfunc(new ImgFunc_shdc01(m_param), m_imgFuncDic, m_imgFuncNames);
	append_imgfunc(new ImgFunc_shdc02(m_param), m_imgFuncDic, m_imgFuncNames);
	append_imgfunc(new ImgFunc_whitening01(m_param), m_imgFuncDic, m_imgFuncNames);
	append_imgfunc(new ImgFunc_whitening02(m_param), m_imgFuncDic, m_imgFuncNames);
	append_imgfunc(new ImgFunc_shdc03(m_param), m_imgFuncDic, m_imgFuncNames);
	append_imgfunc(new ImgFunc_shdc04(m_param), m_imgFuncDic, m_imgFuncNames);
	if (!selectImagingAlgorithmByName(m_imgFuncNames.front())) {
		// (Internal error.)
		throw std::logic_error("*** ERR ***");
	}
}

void ImagingContext::cleanup()
{
	if (m_pImgFunc != NULL) {
		m_pImgFunc->cleanup();
	}
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

/// Set state at once.
void ImagingContext::setState(const int nImgRotAngle, const std::vector<cv::Point>& points)
{
	// Rotate image.
	if (nImgRotAngle < 0) {
		for (int n = -nImgRotAngle; n > 0; n--) {
			rotate(-1);
		}
	}
	else if (nImgRotAngle > 0) {
		for (int n = nImgRotAngle; n > 0; n--) {
			rotate(1);
		}
	}
	assert(m_nImgRotAngle == nImgRotAngle);

	// Set corners.
	clearPointList();
	const size_t sz = points.size();
	for (size_t i = 0; i < sz; i++) {
		const cv::Point srcPt = points[i];
		m_clickedPointList.addOrMovePoint(srcPt);
	}
}

/// 既存座標列が空か否かを返す。
bool ImagingContext::isPointListEmpty() const
{
	return m_clickedPointList.empty();
}

#if 0
/// 既存座標の個数を取得する。
int ImagingContext::pointListSize() const
{
	return m_clickedPointList.size();
}
#endif

#if 0
/// 既存座標列を取得する。
int ImagingContext::getPointList(std::vector<cv::Point>& points) const
{
	return m_clickedPointList.getPointList(points);
}
#endif

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

	// Draw current imaging processing algorithm.
	std::string name = m_pImgFunc->getName();
	std::string summary = m_pImgFunc->getSummary();
	m_imagingCanvas.drawText(name + " -- " + summary);
}

/// キャンバスとソース画像回転
void ImagingContext::rotate(const int dir)
{
	// ソース画像90°回転
	cv::Point ofsAfterRot;
	m_imagingCanvas.rotate(dir, ofsAfterRot);

	// 既存座標リスト内容90°回転
	m_clickedPointList.rotate(dir, ofsAfterRot);

	if (dir < 0) {
		m_nImgRotAngle--;
	}
	else if (dir > 0) {
		m_nImgRotAngle++;
	}
	else {
		/*pass*/
	}
}

/// Get rotation angle of canvas and source image.
int ImagingContext::getImgRotAngle() const
{
	return m_nImgRotAngle;
}

/// Select image processing algorithm.
bool ImagingContext::selectImagingAlgorithmByName(const std::string& name)
{
	auto found_it = m_imgFuncDic.find(name);
	if (found_it == m_imgFuncDic.end()) {
		return false;
	}
	if (m_pImgFunc == NULL || found_it->first != m_pImgFunc->getName()) {
		m_pImgFunc = found_it->second;
	}
	return true;
}

/// Get current image processing algorithm name.
std::string ImagingContext::getCurImagingAlgorithmName() const
{
	return m_pImgFunc->getName();
}

/// Select image processing algorithm.
bool ImagingContext::selectImagingAlgorithmByIdx(const int idx)
{
	if (ZT(idx) >= m_imgFuncNames.size()) {
		return false;
	}
	return selectImagingAlgorithmByName(m_imgFuncNames[ZT(idx)]);
}

/// Get current image processing algorithm index.
int ImagingContext::getCurImagingAlgorithmIdx() const
{
	const auto name = m_pImgFunc->getName();
	auto found_it = std::find(m_imgFuncNames.begin(), m_imgFuncNames.end(), name);
	if (!(found_it != m_imgFuncNames.end())) {
		throw std::logic_error("*** ERR ***");
	}
	auto ofs = std::distance(m_imgFuncNames.begin(), found_it);
	return C_INT(ofs);
}

/// Get number of image processing algorithms.
int ImagingContext::getNumImagingAlgorithms() const
{
	return C_INT(m_imgFuncNames.size());
}

/// 歪み補正
bool ImagingContext::correctDistortion(const cv::Size& dstSz, cv::Mat& dstImg, const bool bConvToGray)
{
	const int nptsExp = 4;

	// グレースケール画像に変換
	cv::Mat targetImage;
	if (bConvToGray) {
		if (!conv_color_to_gray(*m_pSrcImage, targetImage)) {
			return false;
		}
	}
	else {
		targetImage = *m_pSrcImage;
	}

	// If no corner point specified, do resize only.
	if (m_clickedPointList.empty()) {
		cv::resize(targetImage, dstImg, dstSz);
		return true;
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
	warp_image(targetImage, dstImg, srcROICorners2f, nptsExp, dstSz);

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
	cv::Mat BGRImgFromFront;
	if (!correctDistortion(dstSz, BGRImgFromFront, false)) {
		cout << __FUNCTION__ << "correctDistortion() failed." << endl;
		return false;
	}
	assert(BGRImgFromFront.channels() == 3);	// 結果はBGR画像

	// Avoid foreground objects.
	// *(m_param.m_pMaskToAvoidFgObj) willl be overwriten by this subroutine call.
	if (!m_avoidfg.run(BGRImgFromFront, dstImg)) {
		return false;
	}

	cv::Mat gray1;
	cv::cvtColor(BGRImgFromFront, gray1, cv::COLOR_BGR2GRAY);
	m_pImgFunc->dumpImg(gray1, "counter_warped_image");

	// Image processing after perspective correction
	return m_pImgFunc->run(gray1, dstImg);
}
