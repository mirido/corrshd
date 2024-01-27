#include "stdafx.h"
#include "imaging_op.h"
#include "shdcutil.h"

#include "../libnumeric/regression.h"

// [CONF] Add combination product term.
//#define ADD_CMB_PRODUCT

/// Approximate lighting tilt by cubic polynomial.
bool approximate_lighting_tilt_by_cubic_poly(std::vector<LumSample>& samples, std::vector<double>& cflist)
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
	regress_poly_core(G, lum, cflist);

	return true;
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

	// x, x^2, x^3
	pd = x;
	val += (cflist[ZT(1 + 0)] * pd);
	pd *= x;
	val += (cflist[ZT(1 + 1)] * pd);
	pd *= x;
	val += (cflist[ZT(1 + 2)] * pd);

	// y, y^2, y^3
	pd = y;
	val += (cflist[ZT(1 + degree + 0)] * pd);
	pd *= y;
	val += (cflist[ZT(1 + degree + 1)] * pd);
	pd *= y;
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

namespace
{
	void clip_as_lum(double& val)
	{
		if (val < 0.0) {
			val = 0.0;
		}
		else if (val > 255.0) {
			val = 255.0;
		}
		else {
			/*pass*/
		}
	}

}	// namespace

/// Stretch luminance.
void stretch_luminance(cv::Mat& image, const cv::Mat& maskForDLChg, std::vector<double>& cflistOnBg/*, std::vector<double>& cflistOnDL*/)
{
	const int m = image.rows;
	const int n = image.cols;
	for (size_t y = ZT(0); y < ZT(m); y++) {
		for (size_t x = ZT(0); x < ZT(n); x++) {
			uchar& lum = image.at<uchar>(C_INT(y), C_INT(x));		// Alias

			// Masking
			if (maskForDLChg.at<uchar>(C_INT(y), C_INT(x)) == C_UCHAR(0)) {
				//lum = (uchar)255;
				//continue;
			}

			const double fLum = static_cast<double>(lum);

			double whiteLumEnd = predict_by_qubic_poly(cflistOnBg, C_DBL(x), C_DBL(y));
			//double blackLumEnd = predict_by_qubic_poly(cflistOnDL, C_DBL(x), C_DBL(y));

			//if (std::abs(fLum - whiteLumEnd) < std::abs(fLum - blackLumEnd)) {
			//	lum = (uchar)128;
			//	continue;
			//}

			//whiteLumEnd = 255.0 - (255.0 - whiteLumEnd) * 0.8;
			//blackLumEnd *= 0.8;

			//if (fLum >= whiteLumEnd) {
			//	lum = (uchar)255;
			//	continue;
			//}

			//double fNewLum = (255.0 * (fLum - blackLumEnd)) / (whiteLumEnd - blackLumEnd);
			//(void)(blackLumEnd, whiteLumEnd);
			double fNewLum = fLum + (255.0 - whiteLumEnd);
			//double fNewLum = blackLumEnd;
			clip_as_lum(fNewLum);
			lum = static_cast<uchar>(fNewLum);
		}
	}
}
