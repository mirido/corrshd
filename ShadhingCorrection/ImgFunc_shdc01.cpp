#include "stdafx.h"
#include "IImgFunc.h"
#include "ImgFuncBase.h"
#include "ImgFunc_shdc01.h"

#include "../libnumeric/numericutil.h"
#include "../libimaging/geometryutil.h"
#include "../libimaging/imaging_op.h"

ImgFunc_shdc01::ImgFunc_shdc01(Param& param)
	: ImgFuncBase(param), m_whitening01(param)
{
	m_whitening01.needMaskToKeepDrawLine(true);
	m_whitening01.doFinalInversion(false);
}

const char* ImgFunc_shdc01::getName() const
{
	return "shdc01";
}

const char* ImgFunc_shdc01::getSummary() const
{
	return "Shading correction with Black Top Hat, then gamma correction.";
}

bool ImgFunc_shdc01::run(const cv::Mat& srcImg, cv::Mat& dstImg)
{
	cout << std::setbase(10);

	// Whitening with Black Top Hat and get mask to keep draw line.
	cv::Mat gray2;
	if (!m_whitening01.run(srcImg, gray2)) {
		throw std::logic_error("*** ERR ***");
	}
	const cv::Mat maskToKeepDrawLine = m_whitening01.getMaskToKeepDrawLine();

	// 以下、gray2を均一化画像と呼ぶ。

	// 均一化画像gray2平滑化(gray1)
	cv::Mat gray1;
	cv::blur(gray2, gray1, cv::Size(3, 3));
	
	// 均一化画像gray2 (の平滑化結果)のマスクされない画素データを抽出(data)
	const cv::Rect smpROI = get_scaled_rect_from_size(gray1.size(), 1.0);
	std::vector<uchar> data = get_unmasked_data(gray1, maskToKeepDrawLine, smpROI, *(m_param.m_pMaskToAvoidFgObj));
	if (data.size() < 2) {
		return false;
	}

	// dataに対し大津の方法で判別分析を実施
	const int th2 = discriminant_analysis_by_otsu(data);
	cout << "th2=" << th2 << endl;

	// dataの最小値を取得
	const int minv = *std::min_element(data.begin(), data.end());
	cout << "minv=" << minv << endl;

	const int th3 = minv + (int)((double)(th2 - minv) * 0.2);
	cout << "th3=" << th3 << endl;

	// 均一化画像gray2の閾値th3以下を黒(0)、超える画素を原画像のままとする(gray2)
	cv::threshold(gray2, gray2, th3, 255.0, cv::THRESH_TOZERO);
	dumpImg(gray2, "threshold_by_th3");

	// gray2の最大輝度が255になるように調整(gray2)
	double img_minv, img_maxv;
	gray2 = stretch_to_white(gray2, img_minv, img_maxv);
	cout << "img_minv=" << img_minv << endl;
	cout << "img_maxv=" << img_maxv << endl;
	dumpImg(gray2, "stretch_to_white");

	// gray2をガンマ補正(gray2)
	gray2 = gamma_correction(gray2, 3.0);
	dumpImg(gray2, "gumma_correction");

	// gray2を白黒反転(dstImg)
	cv::bitwise_not(gray2, dstImg);

	return true;
}
