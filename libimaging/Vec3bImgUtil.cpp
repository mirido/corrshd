#include "stdafx.h"
#include "Vec3bImgUtil.h"

#include "geometryutil.h"
#include "../libnumeric/numericutil.h"

// [DBGSW] Dump estimated means.
//#define DUMP_ESTM_MEANS
#define DUMP_BEST_MEAN

/// Sample pixels in cv::Vec3b image.
std::vector<cv::Vec3b> sample_Vec3b_pixels(
	const cv::Mat_<cv::Vec3b>& Vec3bImg, const cv::Rect& smpROI, const int cyc_x, const int cyc_y)
{
	const int cntx = get_num_grid_points(smpROI.width, cyc_x);
	const int cnty = get_num_grid_points(smpROI.height, cyc_y);
	const size_t expSz = ZT(cntx) * ZT(cnty);
	std::vector<cv::Vec3b> samples;
	samples.reserve(ZT(expSz));

	int sx, sy, ex, ey;
	decompose_rect(smpROI, sx, sy, ex, ey);
	for (int y = sy; y < ey; y += cyc_y) {
		for (int x = sx; x < ex; x += cyc_x) {
			const cv::Vec3b& pixel = Vec3bImg.at<cv::Vec3b>(y, x);
			samples.push_back(pixel);
		}
	}
	assert(samples.size() == expSz);

	return samples;
}

int get_Vec3b_distance(const cv::Vec3b& a, const cv::Vec3b& b)
{
	return std::abs((int)a[0] - (int)b[0]) + std::abs((int)a[1] - (int)b[1]) + std::abs((int)a[2] - (int)b[2]);
}

cv::Vec3b blend_Vec3b(const cv::Vec3b& a, const cv::Vec3b& b)
{
	cv::Vec3b result(
		C_UCHAR(((int)a[0] + (int)b[0]) / 2),
		C_UCHAR(((int)a[1] + (int)b[1]) / 2),
		C_UCHAR(((int)a[2] + (int)b[2]) / 2)
	);
	return result;
}

void split_near_or_far(
	const std::vector<cv::Vec3b>& samples,
	const cv::Vec3b& center,
	const int sr,
	std::vector<size_t>& lsNear,
	std::vector<size_t>& lsFar
)
{
	const size_t sz = samples.size();
	lsNear.reserve(sz);
	lsFar.reserve(sz);
	for (size_t i = 0; i < sz; i++) {
		const int d = get_Vec3b_distance(center, samples[i]);
		if (d <= sr) {
			lsNear.push_back(i);
		}
		else {
			lsFar.push_back(i);
		}
	}
}

size_t estimate_mean_with_sample_ex(
	const std::vector<cv::Vec3b>& samples, cv::Vec3b& curMean, const int sr)
{
	std::vector<size_t> lsNear, lsFar;
	split_near_or_far(samples, curMean, sr, lsNear, lsFar);

	bool bImproved;
	do {
		bImproved = false;
		for (auto itFar = lsFar.begin(); itFar != lsFar.end(); itFar++) {
			const size_t idxFar = *itFar;
			assert(ZT(0) <= idxFar && idxFar < samples.size());
			int c[3];
			c[0] = samples[idxFar][0];
			c[1] = samples[idxFar][1];
			c[2] = samples[idxFar][2];
			for (auto itNear = lsNear.begin(); itNear != lsNear.end(); itNear++) {
				const size_t idxNear = *itNear;
				assert(ZT(0) <= idxNear && idxNear < samples.size());
				c[0] += samples[idxNear][0];
				c[1] += samples[idxNear][1];
				c[2] += samples[idxNear][2];
			}
			const size_t n = lsFar.size() + ZT(1);
			const cv::Vec3b mean2(C_UCHAR(c[0] / n), C_UCHAR(c[1] / n), C_UCHAR(c[2] / n));

			std::vector<size_t> lsNear2, lsFar2;
			split_near_or_far(samples, mean2, sr, lsNear2, lsFar2);

			if (lsNear2.size() > lsNear.size()) {
				curMean = mean2;
				lsNear = lsNear2;
				lsFar = lsFar2;
				bImproved = true;
				break;
			}
		}
	} while (bImproved);

	return lsNear.size();
}

cv::Vec3b estimate_mean_with_sample(
	const std::vector<cv::Vec3b>& samples, const int sr)
{
	const size_t sz = samples.size();

	size_t max_n = ZT(0);
	cv::Vec3b bestMean = samples.front();
	for (size_t i = 0; i < sz; i ++) {
		cv::Vec3b mean = samples[i];
		const size_t n = estimate_mean_with_sample_ex(samples, mean, sr);
#ifdef DUMP_ESTM_MEANS
		cout << "mean=" << mean << ", n=" << n << endl;
#endif
		if (n > max_n) {
			max_n = n;
			bestMean = mean;
		}
	}

#ifdef DUMP_BEST_MEAN
	cout << "bestMean=" << bestMean << ", n=" << max_n << endl;
#endif
	return bestMean;
}

void make_mask_from_sample(
	const cv::Mat& Vec3bImg, const std::vector<cv::Vec3b>& samples, const int sr, cv::Mat& mask)
{
	const cv::Vec3b mean = estimate_mean_with_sample(samples, sr);

	const cv::Size imgSz = Vec3bImg.size();
	mask = cv::Mat::zeros(imgSz, CV_8UC1);
	for (int y = 0; y < imgSz.height; y++) {
		for (int x = 0; x < imgSz.width; x++) {
			const cv::Vec3b& pixel = Vec3bImg.at<cv::Vec3b>(y, x);
			const int dist = get_Vec3b_distance(pixel, mean);
			if (dist <= sr) {
				mask.at<uchar>(y, x) = C_UCHAR(255);
			}
		}
	}
}
