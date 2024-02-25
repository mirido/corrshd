#include "stdafx.h"
#include "IImgFunc.h"
#include "ImgFuncBase.h"
#include "ImgFuncWithSampling.h"

#include "../libnumeric/regression.h"
#include "../libimaging/shdcutil.h"

ImgFuncWithSampling::ImgFuncWithSampling(Param& param)
	: ImgFuncBase(param)
{
	/*pass*/
}

//
//	For DEBUG
//

/// Dump approximation result visually. (For DEBUG.)
void ImgFuncWithSampling::dumpAppxImg(
	const cv::Mat srcImg,
	const std::vector<double>& cflist,
	const char* const caption
)
{
	if (!m_param.m_pbDump) {
		return;
	}

	const int m = srcImg.rows;
	const int n = srcImg.cols;
	cv::Mat appxImg(m, n, CV_8UC1);
	for (size_t y = ZT(0); y < ZT(m); y++) {
		for (size_t x = ZT(0); x < ZT(n); x++) {
			double apxVal = predict_by_qubic_poly(cflist, C_DBL(x), C_DBL(y));
			if (apxVal < 0.0) {
				apxVal = 0.0;
			}
			else if (apxVal > 255.0) {
				apxVal = 255.0;
			}
			const uchar srcVal = srcImg.at<uchar>(C_INT(y), C_INT(x));
			//appxImg.at<uchar>(C_INT(y), C_INT(x)) = (uchar)(128.0 + 4 * (srcVal - apxVal));
			appxImg.at<uchar>(C_INT(y), C_INT(x)) = (uchar)(255.0 - 2 * std::abs(srcVal - apxVal));
		}
	}
	dumpImg(appxImg, caption);
}

/// Plot sample points. (For DEBUG.)
void ImgFuncWithSampling::plotSamples(
	const cv::Mat_<uchar>& srcImg,
	const std::vector<LumSample>& samples,
	const char* const caption
)
{
	if (!m_param.m_pbDump) {
		return;
	}

	const cv::Vec3b RED{ C_UCHAR(0), C_UCHAR(0), C_UCHAR(255) };

	const cv::Mat kernel = get_bin_kernel(srcImg.size());
	cout << "srcImgSz=" << srcImg.size() << ", kernelSz=" << kernel.size() << endl;

	cv::Mat canvas;
	cv::cvtColor(srcImg, canvas, cv::COLOR_GRAY2BGR);
	//cv::rectangle(canvas, cv::Point(0, 0), cv::Point(canvas.cols - 1, canvas.rows - 1), RED, cv::FILLED);
	{
		cv::Mat thickerImgROI = canvas(cv::Rect(0, 0, kernel.cols, kernel.rows));
		cv::Mat kernelImg = (kernel.clone() * 10 + 128.0);
		cv::Mat kernelImgBGR;
		cv::cvtColor(kernelImg, kernelImgBGR, cv::COLOR_GRAY2BGR);
		kernelImgBGR.copyTo(thickerImgROI);
	}

	const cv::Vec3b markerColor = RED;
	for (auto it = samples.begin(); it != samples.end(); it++) {
		const int x = it->m_point.x;
		const int y = it->m_point.y;
		canvas.at<cv::Vec3b>(y, x) = markerColor;
	}

	dumpImg(canvas, caption);
}
