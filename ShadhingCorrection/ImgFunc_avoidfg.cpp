#include "stdafx.h"
#include "IImgFunc.h"
#include "ImgFuncBase.h"
#include "ImgFunc_avoidfg.h"

#include "../libnumeric/numericutil.h"
#include "../libimaging/shdcutil.h"
#include "../libimaging/Vec3bImgUtil.h"

// [CONF] Max iteration count to decide mask.
#define MAX_ITER_CNT	10

// [CONF] Parameters for cv::pyrMeanShiftFilter().
#define SP		20.0
#define SR		15.0

// [CONF] Ease of accepting disturbances.
#define MAG_TO_ACCEPT_DISTURB	1.0

ImgFunc_avoidfg::ImgFunc_avoidfg(Param& param)
	: ImgFuncBase(param), m_whitening02(param)
{
	m_whitening02.needStdWhiteImg(true);
	m_whitening02.needMaskToKeepDrawLine(false);
	m_whitening02.doFinalInversion(false);
}

const char* ImgFunc_avoidfg::getName() const
{
	return "avoidfg";
}

const char* ImgFunc_avoidfg::getSummary() const
{
	return "Make mask to avoid foreground object.";
}

namespace
{
	void make_mask_to_avoid_fg_obj(const cv::Mat& YUVImg, cv::Mat& maskToAvoidFgObj)
	{
		const cv::Size imgSz = YUVImg.size();
		const int cyc_x = imgSz.width / 10;
		const int cyc_y = imgSz.height / 10;
		auto samples = sample_Vec3b_pixels(YUVImg, cv::Rect(0, 0, imgSz.width, imgSz.height), cyc_x, cyc_y);
		cout << "nsamples=" << samples.size() << endl;
		make_mask_from_sample(YUVImg, samples, C_INT(SR * MAG_TO_ACCEPT_DISTURB + 0.5), maskToAvoidFgObj);
	}

}	// namespace

bool ImgFunc_avoidfg::run(const cv::Mat& srcImg, cv::Mat& dstImg)
{
	// This method assumes that srcImg is BGR image.
	if (srcImg.channels() != 3) {
		throw std::logic_error("*** ERR ***");
	}

	// Make YUV format image of srcImg.
	cv::Mat YUVSrcImg;
	cv::cvtColor(srcImg, YUVSrcImg, cv::COLOR_BGR2YUV);

	// Disable 80% rule.
	*(m_param.m_pRatioOfSmpROIToImgSz) = 1.0;

	// Prepare kernel for morphological operation.
	const cv::Mat kernel = get_bin_kernel(YUVSrcImg.size());

	// Make initial mask to avoid fg obj.
	cv::Mat maskToAvoidFgObj;
	make_mask_to_avoid_fg_obj(YUVSrcImg, maskToAvoidFgObj);
	cv::erode(maskToAvoidFgObj, maskToAvoidFgObj, kernel);		// Extend mask to avoid disturbance pixels.
	dumpImg(maskToAvoidFgObj, "mask to avoid fg obj (1st)");

	// Surpress intermediate image dump.
	const bool sv_bDump = *(m_param.m_pbDump);
	*(m_param.m_pbDump) = false;

	// Make grayscale image of srcImg.
	cv::Mat grayImg;
	cv::cvtColor(srcImg, grayImg, cv::COLOR_BGR2GRAY);

	// Update global mask.
	*(m_param.m_pMaskToAvoidFgObj) = maskToAvoidFgObj;

	// Run whitening02.
	cv::Mat whitened;
	if (!m_whitening02.run(grayImg, whitened)) {
		*(m_param.m_pbDump) = sv_bDump;
		return false;
	}

	// Restore intermediate image dump switch.
	*(m_param.m_pbDump) = sv_bDump;

	// Whiten YUV image with whitening02 result.
	cv::Mat whitenedYUVImg;
	whitenYUVImg(YUVSrcImg, whitenedYUVImg);
	dumpYUVImgAsBGR(whitenedYUVImg, "whitened YUV image (as BGR)");

	// Mean shift filtering to posterize.
	cv::Mat posterized;
	cv::pyrMeanShiftFiltering(whitenedYUVImg, posterized, (SP * whitenedYUVImg.cols) / 640.0, SR, 4);
	dumpYUVImgAsBGR(posterized, "posterized YUV image (as BGR)");

	// Make second mask to avoid fg obj.
	make_mask_to_avoid_fg_obj(posterized, maskToAvoidFgObj);
	cv::erode(maskToAvoidFgObj, maskToAvoidFgObj, kernel);		// Extend mask to avoid disturbance pixels.
	cv::erode(maskToAvoidFgObj, maskToAvoidFgObj, kernel);		// Extend mask to avoid disturbance pixels.
	dumpImg(maskToAvoidFgObj, "mask to avoid fg obj (2nd)");

	// Update global mask (2nd).
	*(m_param.m_pMaskToAvoidFgObj) = maskToAvoidFgObj;

#if 0
	// Run whitening02 (2nd).
	if (!m_whitening02.run(grayImg, dstImg)) {
		return false;
	}
	cv::bitwise_not(dstImg, dstImg);
#else
	dstImg = maskToAvoidFgObj;
#endif

	return true;
}

void ImgFunc_avoidfg::whitenYUVImg(const cv::Mat& YUVSrcImg, cv::Mat& modifiedYUVImg)
{
	modifiedYUVImg = YUVSrcImg.clone();
	cv::Size imgSz = modifiedYUVImg.size();
	const cv::Mat& stdWhiteImg = m_whitening02.getStdWhiteImg();		// Alias
	for (int y = 0; y < imgSz.height; y++) {
		for (int x = 0; x < imgSz.width; x++) {
			cv::Vec3b& pixel = modifiedYUVImg.at<cv::Vec3b>(y, x);		// Alias
			int newY = (int)pixel[0] + (255 - (int)stdWhiteImg.at<uchar>(y, x));
			newY = std::max(0, newY);
			newY = std::min(newY, 255);
			pixel[0] = C_UCHAR(newY);
		}
	}
}

void ImgFunc_avoidfg::dumpYUVImgAsBGR(const cv::Mat& YUVImg, const char* const caption)
{
	if (*m_param.m_pbDump) {
		cv::Mat tmpBGRImg;
		cv::cvtColor(YUVImg, tmpBGRImg, cv::COLOR_YUV2BGR);
		dumpImg(tmpBGRImg, caption);
	}
}
