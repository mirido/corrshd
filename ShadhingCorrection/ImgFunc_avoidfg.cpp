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
#define SR		40.0

// [CONF] Ease of accepting disturbances.
#define MAG_TO_ACCEPT_DISTURB	1.0

// [CONF] Luminance to white (255) margin to be considered white.
#define LUM_MARGIN		3

ImgFunc_avoidfg::ImgFunc_avoidfg(ParamPtr pParam)
	: ImgFuncBase(pParam), m_whitening02(pParam)
{
	m_whitening02.needStdWhiteImg(true);
	m_whitening02.needMaskToKeepDrawLine(false);
	m_whitening02.doFinalInversion(true);
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
		const int cyc_x = (imgSz.width + 9) / 10;
		const int cyc_y = (imgSz.height + 9) / 10;
		auto samples = sample_Vec3b_pixels(YUVImg, cv::Rect(0, 0, imgSz.width, imgSz.height), cyc_x, cyc_y);
		cout << "nsamples=" << samples.size() << endl;
		make_mask_from_sample(YUVImg, samples, C_INT(SR * MAG_TO_ACCEPT_DISTURB + 0.5), maskToAvoidFgObj);
	}

	void add_blured_shadow_area_to_mask(cv::Mat& whitenedGrayImg, cv::Mat& curMask)
	{
		// Prepare kernel for morphological operation.
		const cv::Mat kernel = get_bin_kernel(whitenedGrayImg.size());

		// Erase hand drawing line.
		cv::dilate(whitenedGrayImg, whitenedGrayImg, kernel);
		cv::dilate(whitenedGrayImg, whitenedGrayImg, kernel);
		cv::erode(whitenedGrayImg, whitenedGrayImg, kernel);
		cv::erode(whitenedGrayImg, whitenedGrayImg, kernel);

		// Go arouond edge of image and search shadow.
		const cv::Size imgSz = whitenedGrayImg.size();
		cv::Mat additionMask = cv::Mat::zeros(cv::Size(imgSz.width + 2, imgSz.height + 2), CV_8UC1);
		auto maskAddFunc = [=, &whitenedGrayImg, &additionMask](const cv::Point& seedPt) {
			if (additionMask.at<uchar>(seedPt.y + 1, seedPt.x + 1) <= C_UCHAR(0)) {
				const uchar lum = whitenedGrayImg.at<uchar>(seedPt);
				if (lum < C_UCHAR(240)) {
					// Fill area brightness between 0 and 255 - LUM_MARGIIN.
					// 0 = lum - loDiff, lum + upDiff = (255 - LUM_MARGIN)
					// ==> loDiff = lum, upDiff = (255 - LUM_MARGIN) - lum.
					const double loDiff = (double)lum;
					const double upDiff = (255.0 - (double)LUM_MARGIN) - lum;
					if (loDiff >= 0.0 && upDiff >= 0.0) {
						//cout << "seedPt=" << seedPt << ", lum=" << (double)lum << ", loDiff=" << loDiff << ", upDiff=" << upDiff << endl;
						cv::floodFill(whitenedGrayImg, additionMask, seedPt, cv::Scalar(128.0), 0, cv::Scalar(loDiff), cv::Scalar(upDiff),
							(8 | (255 << 8) | cv::FLOODFILL_MASK_ONLY | cv::FLOODFILL_FIXED_RANGE));
					}
				}
			}
		};
		for (int x = 0; x < imgSz.width; x++) {
			const cv::Point seedPt1(x, 0);
			maskAddFunc(seedPt1);
			const cv::Point seedPt2(x, imgSz.height - 1);
			maskAddFunc(seedPt2);
		}
		for (int y = 0; y < imgSz.height; y++) {
			const cv::Point seedPt1(0, y);
			maskAddFunc(seedPt1);
			const cv::Point seedPt2(imgSz.width - 1, y);
			maskAddFunc(seedPt2);
		}
		cv::bitwise_not(additionMask, additionMask);

		cv::Mat ROIMaskImg = additionMask(cv::Rect(1, 1, imgSz.width, imgSz.height));
		cv::bitwise_and(curMask, ROIMaskImg, curMask);
		cv::erode(curMask, curMask, kernel);
		cv::erode(curMask, curMask, kernel);
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

	// Prepare kernel for morphological operation.
	const cv::Mat kernel = get_bin_kernel(YUVSrcImg.size());

	// Disable 80% rule.
	const int imgLongSideLen = std::max(YUVSrcImg.cols, YUVSrcImg.rows);
	const int smpLongSideLen = std::max(YUVSrcImg.cols - kernel.cols, YUVSrcImg.rows - kernel.rows);
	m_pParam->m_ratioOfSmpROIToImgSz = std::max(0.0, (double)smpLongSideLen / (double)imgLongSideLen);

	// Make initial mask to avoid fg obj.
	cv::Mat maskToAvoidFgObj;
	make_mask_to_avoid_fg_obj(YUVSrcImg, maskToAvoidFgObj);
	cv::erode(maskToAvoidFgObj, maskToAvoidFgObj, kernel);		// Extend mask to avoid disturbance pixels.
	dumpImg(maskToAvoidFgObj, "mask to avoid fg obj (1st)");

	// Surpress intermediate image dump.
	const bool sv_bDump = m_pParam->m_bDump;
	m_pParam->m_bDump = false;

	// Make grayscale image of srcImg.
	cv::Mat grayImg;
	cv::cvtColor(srcImg, grayImg, cv::COLOR_BGR2GRAY);

	// Update global mask.
	m_pParam->m_maskToAvoidFgObj = maskToAvoidFgObj;

	// Run whitening02.
	cv::Mat whitenedGrayImg;
	if (!m_whitening02.run(grayImg, whitenedGrayImg)) {
		m_pParam->m_bDump = sv_bDump;
		return false;
	}

	// Restore intermediate image dump switch.
	m_pParam->m_bDump = sv_bDump;

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
	dumpImg(maskToAvoidFgObj, "mask to avoid fg obj (2nd)");

	// Make blured shadow area to be masked.
	add_blured_shadow_area_to_mask(whitenedGrayImg, maskToAvoidFgObj);
	dumpImg(maskToAvoidFgObj, "mask to avoid fg obj (3rd)");

	// Update global mask (2nd).
	m_pParam->m_maskToAvoidFgObj = maskToAvoidFgObj;

#if 0
	// Run whitening02 (2nd).
	if (!m_whitening02.run(grayImg, dstImg)) {
		return false;
	}
	cv::bitwise_not(dstImg, dstImg);
#else
	// ImgFunc_avoidfg::run() is designed to hage no output image.
	// If we return maskToAvoidFgObj here, the caller will overwrite maskToAvoidFgObj.
	// But Returning maskToAvoidFgObj.clone() is obviously wasteful.
	dstImg = cv::Mat();
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
			clip_as_lum255(newY);
			pixel[0] = C_UCHAR(newY);
		}
	}
}

void ImgFunc_avoidfg::dumpYUVImgAsBGR(const cv::Mat& YUVImg, const char* const caption)
{
	if (m_pParam->m_bDump) {
		cv::Mat tmpBGRImg;
		cv::cvtColor(YUVImg, tmpBGRImg, cv::COLOR_YUV2BGR);
		dumpImg(tmpBGRImg, caption);
	}
}
