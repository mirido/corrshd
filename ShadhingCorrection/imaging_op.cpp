#include "stdafx.h"
#include "imaging_op.h"

/// 画像を歪める。
void warp_image(
	const cv::Mat& srcImage,
	cv::Mat& dstImage,
	const cv::Point2f srcROICorners[],
	const int npts,
	const cv::Size dstSz
)
{
	if (!(npts == 4)) {
		throw std::logic_error("*** ERR ***");
	}

	// 変換後の4点
	const float left = (float)dstSz.width;
	const float btm = (float)dstSz.height;
	cv::Point2f dstPts[4];
	dstPts[0] = cv::Point2f(0.0F, 0.0F);
	dstPts[1] = cv::Point2f(left, 0.0F);
	dstPts[2] = cv::Point2f(left, btm);
	dstPts[3] = cv::Point2f(0.0f, btm);

	// 変換行列取得
	const cv::Mat M = cv::getPerspectiveTransform(srcROICorners, dstPts);

	// 変換実施
	cv::warpPerspective(srcImage, dstImage, M, dstSz);
}
