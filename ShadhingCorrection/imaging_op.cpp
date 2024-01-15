#include "stdafx.h"
#include "imaging_op.h"

/// �摜��c�߂�B
void warp_image(
	const cv::Mat& srcImage,
	const cv::Mat& dstImage,
	const cv::Point2f srcPts[],
	const int npts,
	const cv::Size dstSz
)
{
	if (!(npts == 4)) {
		throw std::logic_error("*** ERR ***");
	}

	// �ϊ����4�_
	const float left = (float)dstSz.width;
	const float btm = (float)dstSz.height;
	cv::Point2f dstPts[4];
	//dstPts[0] = cv::Point2f(0.0F, 0.0F) + cv::Point2f(100.0F, 100.0F);
	//dstPts[1] = cv::Point2f(left, 0.0F) + cv::Point2f(100.0F, 100.0F);
	//dstPts[2] = cv::Point2f(left, btm) + cv::Point2f(100.0F, 100.0F);
	//dstPts[3] = cv::Point2f(0.0f, btm) + cv::Point2f(100.0F, 100.0F);
	for (int i = 0; i < 4; i++) {
		dstPts[i] = srcPts[i];
	}

	// �ϊ��s��擾
	const cv::Mat M = cv::getPerspectiveTransform(srcPts, dstPts);

	// �ϊ����{
	cv::warpPerspective(srcImage, dstImage, M, dstSz);
}
