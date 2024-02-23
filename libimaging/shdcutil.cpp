#include "stdafx.h"
#include "imaging_op.h"
#include "shdcutil.h"

#include "../libnumeric/numericutil.h"
#include "../libnumeric/regression.h"
#include "geometryutil.h"

// [CONF] Add combination product term.
//#define ADD_CMB_PRODUCT
// Don't define the above macro ADD_CMB_PRODUCT.
// If defined, shading correction will fail because the luminance is
// fitted to the product of x and y instead of the coordinate (x, y).

/// Sample pixels in image.
std::vector<LumSample> sample_pixels(
	const cv::Mat_<uchar>& image,
	const cv::Rect& smpROI,
	const int cyc_x,
	const int cyc_y,
	const cv::InputArray globalMask
)
{
	const int cntx = get_num_grid_points(smpROI.width, cyc_x);
	const int cnty = get_num_grid_points(smpROI.height, cyc_y);
	const size_t expSz = ZT(cntx) * ZT(cnty);
	std::vector<LumSample> samples;
	samples.reserve(ZT(expSz));

	int sx, sy, ex, ey;
	decompose_rect(smpROI, sx, sy, ex, ey);
	if (globalMask.empty()) {
		for (int y = sy; y < ey; y += cyc_y) {
			for (int x = sx; x < ex; x += cyc_x) {
				const uchar lum = image.at<uchar>(y, x);
				samples.push_back(LumSample(x, y, lum));
			}
		}
		assert(samples.size() == expSz);
	}
	else {
		const cv::Mat gmask = globalMask.getMat();
		if (!(gmask.size() == image.size())) {
			throw std::logic_error("*** ERR ***");
		}
		size_t ignoredCnt = ZT(0);
		for (int y = sy; y < ey; y += cyc_y) {
			for (int x = sx; x < ex; x += cyc_x) {
				if (gmask.at<uchar>(y, x) > C_UCHAR(0)) {
					const uchar lum = image.at<uchar>(y, x);
					samples.push_back(LumSample(x, y, lum));
				}
				else {
					ignoredCnt++;
				}
			}
		}
		assert(samples.size() + ignoredCnt == expSz);
	}

	return samples;
}

/// Approximate lighting tilt by cubic polynomial.
bool approximate_lighting_tilt_by_cubic_poly(const std::vector<LumSample>& samples, std::vector<double>& cflist)
{
	double pd;

	const int degree = 3;
	const int m = C_INT(samples.size());
#ifdef ADD_CMB_PRODUCT
	const int n = 1 + 2 * degree + 3;
#else
	const int n = 1 + 2 * degree;
#endif

	if (m <= 0) {
		return false;
	}

	// Make design matrix.
	cv::Mat G(m, n, CV_64FC1);
	for (size_t i = ZT(0); i < ZT(m); i++) {
		const double x = C_DBL(samples[i].m_point.x);
		const double y = C_DBL(samples[i].m_point.y);

		// Constant
		G.at<double>(C_INT(i), 0) = 1.0;

		// y, y^2, y^3
		pd = 1.0;
		for (size_t j = 0; j < degree; j++) {
			pd *= y;
			G.at<double>(C_INT(i), 1 + C_INT(j)) = pd;
		}

		// x, x^2, x^3
		pd = 1.0;
		for (size_t j = 0; j < degree; j++) {
			pd *= x;
			G.at<double>(C_INT(i), 1 + degree + C_INT(j)) = pd;
		}

#ifdef ADD_CMB_PRODUCT
		// y*x, y*(x^2), (y^2)*x
		pd = x * y;
		G.at<double>(C_INT(i), 1 + 2 * degree + 0) = pd;
		G.at<double>(C_INT(i), 1 + 2 * degree + 1) = pd * x;
		G.at<double>(C_INT(i), 1 + 2 * degree + 2) = pd * y;
#endif
	}

	// Make response matrix.
	cv::Mat lum(m, 1, CV_64FC1);
	for (size_t i = ZT(0); i < ZT(m); i++) {
		lum.at<double>(C_INT(i), 0) = C_DBL(samples[i].m_lum);
	}

	// Solve.
	//cout << "G=" << G << endl;
	//cout << "lum=" << lum << endl;
	return regress_poly_core(G, lum, cflist);
}

/// Predict by cubic polynomial.
double predict_by_qubic_poly(const std::vector<double>& cflist, const double x, const double y)
{
	double pd;

	const int degree = 3;
#ifdef ADD_CMB_PRODUCT
	assert(cflist.size() == ZT(1 + 2 * degree + 3));
#else
	assert(cflist.size() == ZT(1 + 2 * degree));
#endif

	// Constant
	double val = cflist[0];

	// y, y^2, y^3
	pd = y;
	val += (cflist[ZT(1 + 0)] * pd);
	pd *= y;
	val += (cflist[ZT(1 + 1)] * pd);
	pd *= y;
	val += (cflist[ZT(1 + 2)] * pd);

	// x, x^2, x^3
	pd = x;
	val += (cflist[ZT(1 + degree + 0)] * pd);
	pd *= x;
	val += (cflist[ZT(1 + degree + 1)] * pd);
	pd *= x;
	val += (cflist[ZT(1 + degree + 2)] * pd);

#ifdef ADD_CMB_PRODUCT
	// y*x, y*(x^2), (y^2)*x
	pd = x * y;
	val += (cflist[ZT(1 + 2 * degree + 0)] * pd);
	pd *= y;
	val += (cflist[ZT(1 + 2 * degree + 1)] * (pd * x));
	pd *= y;
	val += (cflist[ZT(1 + 2 * degree + 2)] * (pd * y));
#endif

	return val;
}

/// Predict while image.
void predict_image(const cv::Size& imgSz, const std::vector<double>& cflist, cv::Mat& dstImg)
{
	const int m = imgSz.height;
	const int n = imgSz.width;

	dstImg = cv::Mat(m, n, CV_8UC1);
	for (int y = 0; y < m; y++) {
		for (int x = 0; x < n; x++) {
			double fLum = predict_by_qubic_poly(cflist, C_DBL(x), C_DBL(y));
			clip_as_lum255(fLum);
			dstImg.at<uchar>(y, x) = static_cast<uchar>(fLum);
		}
	}
}

/// Stretch and invert luminance.
void stretch_and_invert_luminance(cv::Mat& image, const cv::Mat& maskForDLChg, const cv::Mat& invBlacknessMap)
{
	const int m = image.rows;
	const int n = image.cols;
	if (!(maskForDLChg.rows == m && maskForDLChg.cols == n)) {
		throw std::logic_error("*** ERR ***");
	}
	if (!(invBlacknessMap.rows == m && invBlacknessMap.cols == n)) {
		throw std::logic_error("*** ERR ***");
	}

	for (int y = 0; y < m; y++) {
		for (int x = 0; x < n; x++) {
			uchar& lum = image.at<uchar>(y, x);		// Alias
			double fLum = static_cast<double>(lum);

			if (maskForDLChg.at<uchar>(y, x) == C_UCHAR(0)) {
				// (Point on background)
				fLum = 255.0;				// Mask background
				//fLum = 255.0 - fLum;		// Inversion only on background.
			}
			else {
				// (Point near drawing line)
				const double invBlackLumEnd = C_DBL(invBlacknessMap.at<uchar>(y, x));
				//fLum = invBlackLumEnd;		// Test.
				//fLum = 1.0;				// Test.
				//fLum = 0.0;				// Test.
				if (invBlackLumEnd != 0.0) {
					fLum = 255.0 * (1.0 - fLum / invBlackLumEnd);
				}
				else {
					fLum = 0.0;
				}
			}

			clip_as_lum255(fLum);
			lum = static_cast<uchar>(fLum);
		}
	}
}

/// Stretch luminance.
void stretch_luminance(cv::Mat& image, const cv::Mat& lumEndMap)
{
	const int m = image.rows;
	const int n = image.cols;
	if (!(lumEndMap.rows == m && lumEndMap.cols == n)) {
		throw std::logic_error("*** ERR ***");
	}

	for (int y = 0; y < m; y++) {
		for (int x = 0; x < n; x++) {
			uchar& lum = image.at<uchar>(y, x);		// Alias
			double fLum = static_cast<double>(lum);

			const double lumEnd = C_DBL(lumEndMap.at<uchar>(y, x));
			if (lumEnd != 0.0) {
				fLum = (255.0 * fLum) / lumEnd;
			}
			else {
				fLum = 255.0;
			}
			clip_as_lum255(fLum);
			lum = static_cast<uchar>(fLum);
		}
	}
}
