#include "stdafx.h"
#include "regression.h"

/// Do polynomial regression.
void regress_poly_ex(
	const std::function<double(size_t)> xlistFunc,
	const std::function<double(size_t)> ylistFunc,
	const size_t nsamples,
	const int dim,
	cv::Mat& cflist
)
{
	// The comment shows the case of finding a quadratic curve (2rd order curve)
	// passing through points (x, y) = (1, 2), (3, 4), (5, 6), (7, 8).
	// Input:
	//   xlistFunc(k) = function that returns ( 1, 3, 5, 7 )
	//   ylistFunc(k) = function that returns ( 2, 4, 6, 8 )
	//   nsamples = 4
	//   dim   = 2

	// Make response vector y. (List of y coordinates.)
	const int n = C_INT(nsamples);
	cv::Mat y(n, 1, CV_64FC1);
	for (size_t i = ZT(0); i < ZT(n); i++) {
		y.at<double>(C_INT(i), 0) = ylistFunc(i);
	}
	// Now
	//   n = 4                  // nsamples
	//   y = (  2  4  6  8 )^T

	// Make design matrix G
	const int m = dim + 1;
	cv::Mat G(n, m, CV_64FC1);
	for (size_t i = ZT(0); i < ZT(n); i++) {
		const double x = xlistFunc(i);
		G.at<double>(C_INT(i), 0) = 1.0;
		for (size_t j = ZT(1); i < ZT(m); j++) {
			G.at<double>(C_INT(i), C_INT(j)) = G.at<double>(C_INT(i), C_INT(j - 1)) * x;
		}
	}
	// Now
	//   n = 3                  // nrows    (= nsamples)
	//   m = 3                  // ncolumns (= dim + 1)
	//   G = (  1  1  1         // = ( 1^0, 1^1, 1^2 )
	//          1  3  9         // = ( 3^0, 3^1, 3^2 )
	//          1  5 25         // = ( 5^0, 5^1, 5^2 )
	//          1  7 49 )       // = ( 7^0, 7^1, 7^2 )

	// We have to minimize the following norm:
	//   E(cflist) = |y - (G)(cflist)|^2
	//             = ((y - (G)(cflist))^T)(y - (G)(cflist))
	//             = (y^T - ((cflist)^T)(G^T))(y - (G)(cflist))
	//             = (y^T)(y) - ((cflist)^T)(G^T)(y) - (y^T)(G)(cflist) + ((cflist)^T)(G^T)(G)(cflist)
	//             = (y^T)(y) - 2((cflist)^T)(G^T)(y) + ((cflist)^T)(G^T)(G)(cflist)
	// To do this, we need to find the cflist that makes differential coefficient 0.
	// This means we have to solve the following equation:
	//   (0) = -2(G^T)(y) + 2(G^T)(G)(cflist)
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
	if (!(GTG.rows == m && GTG.cols == m)) {
		throw std::logic_error("*** ERR ***");
	}
	if (!(GTy.rows == m && GTy.cols == 1)) {
		throw std::logic_error("*** ERR ***");
	}
	cv::solve(GTG, GTy, cflist, cv::DECOMP_LU);
}

/// Do polynomial approximation. (cv::Point interface)
void regress_poly(const std::vector<cv::Point>& points, const int dim, std::vector<double>& cflist)
{
	auto xlistFunc = [&points](size_t k)->int { return points[k].x; };
	auto ylistFunc = [&points](size_t k)->int { return points[k].y; };
	const size_t nsamples = points.size();

	cv::Mat cfs;
	regress_poly_ex(xlistFunc, ylistFunc, nsamples, dim, cfs);

	if (!(cfs.cols == 1)) {
		throw std::logic_error("*** ERR ***");
	}
	const size_t sz = ZT(cfs.rows);
	cflist.resize(sz);
	for (size_t i = 0; i < sz; i++) {
		cflist[i] = cfs.at<double>(C_INT(i), 0);
	}
}
