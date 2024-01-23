#include "stdafx.h"
#include "imaging_op.h"
#include "shdcutil.h"

#include "../libnumeric/regression.h"

/// Approximate lighting tilt by cubic equation.
void approximate_lighting_tilt_by_cubic_eqn(std::vector<LumSample>& samples, std::vector<double>& cflist)
{
	double pd;

	const int degree = 3;

	const int m = C_INT(samples.size());
	const int n = 1 + degree + 3;

	// Make design matrix.
	cv::Mat G(m, n, CV_64FC1);
	for (size_t i = ZT(0); i < ZT(m); i++) {
		const double x = samples[i].m_point.x;
		const double y = samples[i].m_point.y;

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

		// y*x, y*(x^2), (y^2)*x
		pd = x * y;
		G.at<double>(C_INT(i), 1 + 2 * degree + 0) = pd;
		G.at<double>(C_INT(i), 1 + 2 * degree + 1) = pd * x;
		G.at<double>(C_INT(i), 1 + 2 * degree + 2) = pd * y;
	}

	// Make response matrix.
	cv::Mat lum(m, 1, CV_64FC1);
	for (size_t i = ZT(0); i < ZT(m); i++) {
		lum.at<double>(C_INT(i), 0) = C_DBL(samples[i].m_lum);
	}

	// Solve.
	regress_poly_core(G, lum, cflist);
}
