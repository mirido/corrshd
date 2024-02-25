#include "stdafx.h"
#include "IImgFunc.h"
#include "ImgFuncBase.h"
#include "ImgFunc_uniform.h"

#include "../libnumeric/numericutil.h"
#include "../libimaging/shdcutil.h"

ImgFunc_uniform::ImgFunc_uniform(Param& param)
	: ImgFuncWithSampling(param)
{
	/*pass*/
}

const char* ImgFunc_uniform::getName() const
{
	return "uniforming";
}

const char* ImgFunc_uniform::getSummary() const
{
	return "Uniform peak level of pixels.";
}

namespace
{
	/// Predict while image.
	bool add_artificial_samples_to_modify_uncorrectables(
		const cv::Size& imgSz,
		const cv::Size& kernelSz,
		const std::vector<double>& cflist,
		std::vector<LumSample>& samplesOnDL
	)
	{
		// Get mean and stddev.
		const int n = C_INT(samplesOnDL.size());
		cv::Mat data(cv::Size(n, 1), CV_32SC1);
		for (int j = 0; j < n; j++) {
			data.at<int>(0, j) = C_INT(samplesOnDL[j].m_lum);
		}
		const cv::Scalar mean, stddev;
		cv::meanStdDev(data, mean, stddev);
		const double fmean = mean[0];
		const double fstddev = stddev[0];
		auto sv_flags = cout.flags();
		cout << "  mean=" << std::showpos << fmean << endl;
		cout << "stddev=" << std::showpos << fstddev << endl;
		cout.flags(sv_flags);

		// Decide minimum value of black level.
		//const double minBlackLevel = 0.0;
		//const double minBlackLevel = std::max(0.0, fmean);
		const double minBlackLevel = std::max(0.0, fmean - fstddev * 0.7);
		//const double minBlackLevel = std::max(0.0, fmean - fstddev);
		cout << "minBlackLevel=" << minBlackLevel << endl;

		auto addSmpIfFunc = [=, &cflist, &samplesOnDL](const cv::Point& pt) {
			double fLum = predict_by_qubic_poly(cflist, C_DBL(pt.x), C_DBL(pt.y));

			// For fLum, 0 means white, 255 means black.
			// The fLum on (x, y) will be stretched in propotion to ratio of [0, 255] to [0, flum]. 
			// Therefore, if fLum is too small, the value cannot be determined.
			if (fLum <= C_DBL(minBlackLevel)) {
				LumSample val;
				val.m_point = pt;
				val.m_lum = C_UCHAR(minBlackLevel);
				samplesOnDL.push_back(val);
				return true;
			}
			return false;
		};

		const int sx = kernelSz.width / 2;
		const int sy = kernelSz.height / 2;
		const int cyc_x = kernelSz.width;
		const int cyc_y = kernelSz.height;

		bool bChanged = false;
		for (int y = sy; y < imgSz.height; y += cyc_y) {
			if (addSmpIfFunc(cv::Point(0, y))) {
				bChanged = true;
			}
			if (addSmpIfFunc(cv::Point(imgSz.width - 1, y))) {
				bChanged = true;
			}
		}
		for (int x = sx; x < imgSz.width; x += cyc_x) {
			if (addSmpIfFunc(cv::Point(x, 0))) {
				bChanged = true;
			}
			if (addSmpIfFunc(cv::Point(x, imgSz.height - 1))) {
				bChanged = true;
			}
		}

		return bChanged;
	}

}	// namespace

bool ImgFunc_uniform::run(const cv::Mat& srcImg, cv::Mat& dstImg)
{
	// This method assumes that background pixels of srcImg are almost equalized to 0.

	// Make mask to keep draw line.
	makeMaskToKeepDrawLine(srcImg, *(m_param.m_pRatioOfSmpROIToImgSz), *(m_param.m_pMaskToAvoidFgObj));
	cv::Mat maskToKeepDL = m_maskToKeepDrawLine;

	// Prepare kernel for dirate or erode.
	const cv::Mat kernel = get_bin_kernel(srcImg.size());

	// Sample pixels on drawing line.
	cv::Mat morphoTmpImg;
	cv::dilate(srcImg, morphoTmpImg, kernel);
	auto samplesOnDL = sampleDrawLine(morphoTmpImg, maskToKeepDL);

	maskToKeepDL.release();

	// Approximage blackness tilt on drawing line (1st).
	std::vector<double> cflistOnDL;
	if (!approximate_lighting_tilt_by_cubic_poly(samplesOnDL, cflistOnDL)) {
		return false;
	}

	// Check uncorrectable pixels exist or not.
	const size_t sv_nsamples = samplesOnDL.size();
	if (add_artificial_samples_to_modify_uncorrectables(srcImg.size(), kernel.size(), cflistOnDL, samplesOnDL)) {
		cout << "nsamples on drawing line changed. (" << sv_nsamples << " --> " << samplesOnDL.size() << ")" << endl;

		// Approximage blackness tilt on drawing line (2nd).
		if (!approximate_lighting_tilt_by_cubic_poly(samplesOnDL, cflistOnDL)) {
			return false;
		}
	}
	else {
		assert(samplesOnDL.size() == sv_nsamples);
	}

	cout << "samplesOnDL: size=" << samplesOnDL.size() << endl;
	plotSamples(morphoTmpImg, samplesOnDL, "samples on drawing line");
	morphoTmpImg.release();

	// Enhance drawing line.
	cv::Mat invBlacknessTiltImg;
	predict_image(srcImg.size(), cflistOnDL, invBlacknessTiltImg);
	dumpImg(invBlacknessTiltImg, "inv blackness tilt image");
	dstImg = srcImg;
	stretch_luminance(dstImg, invBlacknessTiltImg);

	return true;
}

/// Sample pixels on drawing line. 
std::vector<LumSample> ImgFunc_uniform::sampleDrawLine(
	const cv::Mat_<uchar>& image,
	const cv::Mat_<uchar>& maskToKeepDL
)
{
	const cv::Size kernelSz = get_bin_kernel_size(image.size());
	const cv::Rect smpROI = get_bin_ROI(image.size(), *(m_param.m_pRatioOfSmpROIToImgSz));

	const int cntx = get_num_grid_points(smpROI.width, kernelSz.width);
	const int cnty = get_num_grid_points(smpROI.height, kernelSz.height);
	const size_t nsamples = ZT(cntx) * ZT(cnty);

	auto samplesOnDL = get_unmasked_point_and_lum(image, maskToKeepDL, smpROI, *(m_param.m_pMaskToAvoidFgObj));

	const size_t sz = samplesOnDL.size();
	if (sz <= nsamples) {
		return samplesOnDL;
	}

	const size_t cyc = (sz + (nsamples - ZT(1))) / nsamples;
	const size_t expSz = (sz + (cyc - ZT(1))) / cyc;
	std::vector<LumSample> smpDL;
	smpDL.reserve(expSz);
	for (size_t i = 0; i < sz; i += cyc) {
		smpDL.push_back(samplesOnDL[i]);
	}
	assert(smpDL.size() == expSz);

	return smpDL;
}
