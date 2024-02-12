#include "stdafx.h"

#include "../libimaging/imaging_op.h"
#include "../libimaging/shdcutil.h"

#include "../libimaging/geometryutil.h"
#include "../libnumeric/numericutil.h"

// [DBGSW] Show intermediate image.
//#define SHOW_ITM_IMG

#ifdef NDEBUG
#undef SHOW_ITM_IMG
#endif

const int IMG_WIDTH = 600;
const int IMG_HEIGHT = 800;
const double SMP_ROI_RATIO = 0.8;
const int SMP_CYC = (int)std::round(IMG_WIDTH * 0.025);

namespace
{
	void generate_predict_image(const cv::Size& imgSz, const std::vector<double>& cflist, cv::Mat& dstImg)
	{
		dstImg = cv::Mat(imgSz, CV_8UC1);

		const int m = dstImg.rows;
		const int n = dstImg.cols;

		for (int y = 0; y < m; y++) {
			for (int x = 0; x < n; x++) {
				double lum = predict_by_qubic_poly(cflist, x, y);
				clip_as_lum255(lum);
				dstImg.at<uchar>(y, x) = static_cast<uchar>(lum);
			}
		}
	}

	void make_test_image(cv::Mat& image)
	{
		bool bRet;

		image = cv::Mat(IMG_HEIGHT, IMG_WIDTH, CV_8UC1);

		const int w = image.cols;
		const int h = image.rows;

		const std::vector<LumSample> samples{
			LumSample(0, 0, C_UCHAR(0)),
			LumSample(w - 1, 0, C_UCHAR(80)),
			LumSample(w / 2, h / 2, C_UCHAR(128)),
			LumSample(0, h - 1, C_UCHAR(160)),
			LumSample(w - 1, h - 1, C_UCHAR(255)),
		};

		std::vector<double> cflist;
		bRet = approximate_lighting_tilt_by_cubic_poly(samples, cflist);
		if (!bRet) {
			throw std::logic_error("*** ERR ***");
		}

		generate_predict_image(image.size(), cflist, image);
	}

	/// Common subroutine of approximation and predict test.
	void do_approximage_and_predict(const cv::Mat& image, cv::Mat& dstImg)
	{
		bool bRet;

		// Sample pixels.
		const cv::Rect smpROI = get_scaled_rect_from_size(image.size(), SMP_ROI_RATIO);
		cout << "smpROI=" << smpROI << ", SMP_CYC=" << SMP_CYC << endl;
		std::vector<LumSample> samples = sample_pixels(image, smpROI, SMP_CYC, SMP_CYC);
		ASSERT_GT(samples.size(), ZT(0));
		const int expSz = ((smpROI.width + (SMP_CYC - 1)) / SMP_CYC) * ((smpROI.height + (SMP_CYC - 1)) / SMP_CYC);
		EXPECT_EQ(ZT(expSz), samples.size());

		// Approximate.
		std::vector<double> cflist;
		bRet = approximate_lighting_tilt_by_cubic_poly(samples, cflist);
		ASSERT_TRUE(bRet);
		EXPECT_EQ(7, cflist.size());

		// Predict.
		generate_predict_image(image.size(), cflist, dstImg);
	}

	/// Evaluate error between imgExp and imgAct.
	void evaluate_error(const cv::Mat& imgExp, const cv::Mat& imgAct)
	{
		cv::Mat signedDiffImg;
		{
			cv::Mat diffImg = imgAct - imgExp;
			diffImg.convertTo(signedDiffImg, CV_16SC1, 1.0, 0.0);
		}

		cv::Mat mean, stddev;
		cv::meanStdDev(signedDiffImg, mean, stddev);
		cout << "mean.size()=" << mean.size() << ", stddev.size()=" << stddev.size() << endl;
		const double fmean = mean.at<double>(0, 0);
		const double fstddev = stddev.at<double>(0, 0);
		cout << "  mean=" << fmean << endl;
		cout << "stddev=" << fstddev << endl;
		EXPECT_TRUE(can_equal(fmean, 0.0));
		EXPECT_TRUE(can_equal(fstddev, 0.0));
	}

}	// namespace

TEST(stdcutilTest, plane_image01) {
	SCOPED_TRACE(__FUNCTION__);

	// Make source srcImg.
	cv::Mat srcImg = cv::Mat::zeros(cv::Size(IMG_HEIGHT, IMG_WIDTH), CV_8UC1);
	srcImg += 128.0;
#ifdef SHOW_ITM_IMG
	cv::imshow(__FUNCTION__, srcImg);
	cv::waitKey(0);
	cv::destroyWindow(__FUNCTION__);
#endif

	// Get predict srcImg.
	cv::Mat dstImg;
	do_approximage_and_predict(srcImg, dstImg);

	// Evaluate error.
	evaluate_error(srcImg, dstImg);
}

TEST(stdcutilTest, plane_image02) {
	SCOPED_TRACE(__FUNCTION__);

	// Make source srcImg.
	cv::Mat srcImg = cv::Mat(cv::Size(IMG_HEIGHT, IMG_WIDTH), CV_8UC1);
	const int m = srcImg.rows;
	const int n = srcImg.cols;
	const int den = (m - 1) + (n - 1);
	for (int y = 0; y < m; y++) {
		for (int x = 0; x < n; x++) {
			const double fLum = (C_DBL(x + y) * 255.0) / C_DBL(den);
			srcImg.at<uchar>(y, x) = static_cast<uchar>(fLum);
		}
	}
#ifdef SHOW_ITM_IMG
	cv::imshow(__FUNCTION__, srcImg);
	cv::waitKey(0);
	cv::destroyWindow(__FUNCTION__);
#endif

	// Get predict srcImg.
	cv::Mat dstImg;
	do_approximage_and_predict(srcImg, dstImg);

	// Evaluate error.
	evaluate_error(srcImg, dstImg);
}

TEST(stdcutilTest, curved_image) {
	SCOPED_TRACE(__FUNCTION__);

	// Make source srcImg.
	cv::Mat srcImg;
	make_test_image(srcImg);

	// Get predict srcImg.
	cv::Mat dstImg;
	do_approximage_and_predict(srcImg, dstImg);
#ifdef SHOW_ITM_IMG
	cv::imshow(__FUNCTION__ " src", srcImg);
	cv::imshow(__FUNCTION__ " dst", dstImg);
	cv::waitKey(0);
	try {
		cv::destroyWindow(__FUNCTION__ " src");
		cv::destroyWindow(__FUNCTION__ " dst");
	}
	catch (cv::Exception& e) {
		const char* const msg = e.what();
		cout << "Warning: " << msg << endl;
	}
#endif

	// Evaluate error.
	evaluate_error(srcImg, dstImg);
}
