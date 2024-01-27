#include "stdafx.h"
#include "regression.h"

/// Do polynomial regression.
bool regress_poly_core(
	const cv::Mat& G,
	const cv::Mat& y,
	cv::Mat& cflist
)
{
	// The comment shows the case of finding a quadratic curve (2rd order curve)
	// passing through points (x, y) = (1, 2), (3, 4), (5, 6), (7, 8).
	// Input:
	//   G = (  1  1  1         // = ( 1^0, 1^1, 1^2 )
	//          1  3  9         // = ( 3^0, 3^1, 3^2 )
	//          1  5 25         // = ( 5^0, 5^1, 5^2 )
	//          1  7 49 )       // = ( 7^0, 7^1, 7^2 )
	//   y = (  2  4  6  8 )^T

	const int m = G.rows;		// Number of samples
	const int n = G.cols;		// Number of coefficients (= (degree of the polynomial) + 1)

	if (!(y.rows == m && y.cols == 1)) {
		throw std::logic_error("*** ERR ***");
	}

	// We have to minimize the following norm:
	//   E(cflist) = |y - (G)(cflist)|^2
	//             = |y|^2 - 2<y, (G)(cflist)> + |(G)(cflist)|^2
	//             = (y^T)(y) - 2(y^T)(G)(cflist) + ((cflist)^T)(G^T)(G)(cflist)
	// To do this, we need to find the cflist that makes differential coefficient 0.
	// This means we have to solve the following equation:
	//   (0) = -2(y^T)(G) + 2(G^T)(G)(cflist)
	// That is:
	//   (G^T)(G)(cflist) = (G^T)(y)

	cv::Mat GT = G.t();		    // = G^T (Transposed matrix of G)
	cv::Mat GTy = GT * y;	    // = (G^T)y
	// Now
	//   GT = (  1  1  1  1
	//           1  3  5  7
	//           1  9 15 49 )
	//   GTy = (  20            // = ( 1 1  1  1 )( 2 4 6 8 )^T = 2 +  4 +  6 +   8
	//           100            // = ( 1 3  5  7 )( 2 4 6 8 )^T = 2 + 12 + 30 +  56
	//           520 )          // = ( 1 9 15 49 )( 2 4 6 8 )^T = 2 + 36 + 90 + 392

	cv::Mat GTG = GT * G;	    // (G^T)(G)
	// Now we have to solve tha eauation:
	//   (GTG)(cflist) = GTy

	// Solve
	assert(GTG.rows == n && GTG.cols == n);
	assert(GTy.rows == n && GTy.cols == 1);
	if (!cv::solve(GTG, GTy, cflist, cv::DECOMP_LU)) {
		return false;
	}
	assert(cflist.cols == 1);

	return true;
}

/// Do polynomial regression. (Output to std::vector<double>)
bool regress_poly_core(
	const cv::Mat& G,
	const cv::Mat& y,
	std::vector<double>& cflist
)
{
	// Get parameter vector cfs.
	cv::Mat cfs;
	if (!regress_poly_core(G, y, cfs)) {
		return false;
	}
	assert(cfs.cols == 1);

	// Expand cfs to std::vector<double> cflist.
	const size_t sz = ZT(cfs.rows);
	cflist.resize(sz);
	for (size_t i = 0; i < sz; i++) {
		cflist[i] = cfs.at<double>(C_INT(i), 0);
	}

	return true;
}

/// Do polynomial regression. (cv::Point interface)
bool regress_poly(const std::vector<cv::Point2d>& points, const int degree, std::vector<double>& cflist)
{
	const int m = C_INT(points.size());
	const int n = degree + 1;

	// Make design matrix G.
	cv::Mat G(m, n, CV_64FC1);
	for (size_t i = ZT(0); i < ZT(m); i++) {
		const double x = points[i].x;
		G.at<double>(C_INT(i), 0) = 1.0;
		for (size_t j = ZT(1); j < ZT(n); j++) {
			G.at<double>(C_INT(i), C_INT(j)) = G.at<double>(C_INT(i), C_INT(j - 1)) * x;
		}
	}

	// Make response vector y.
	cv::Mat y(m, 1, CV_64FC1);
	for (size_t i = ZT(0); i < ZT(m); i++) {
		y.at<double>(C_INT(i), 0) = points[i].y;
	}

	// Get parameter vector cflist.
	return regress_poly_core(G, y, cflist);
}
