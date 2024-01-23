#include "stdafx.h"
#include "../libnumeric/regression.h"

TEST(regressionTest, least_squares_method) {
	std::vector<cv::Point2d> points{
		{ 116.5, 21.3 },
		{ 125.5, 22.0 },
		{ 128.1, 26.9 },
		{ 132.0, 32.3 },
		{ 141.0, 33.1 },
		{ 145.2, 38.2 },
	};

	std::vector<double> cflist;
	regress_poly(points, 1, cflist);
	ASSERT_EQ((size_t)2, cflist.size());

	std::ostringstream ost;
	ost.precision(ZT(4));
	ost << std::fixed << "y = " << cflist[1] << " * x + (" << cflist[0] << ")";
	const std::string eqn = ost.str();
	EXPECT_EQ(std::string("y = 0.6016 * x + (-50.0675)"), eqn);
}

#ifdef USE_OPENCV_WORLD_DLL
#ifdef NDEBUG
#pragma comment(lib, "opencv_world480.lib")
#else
#pragma comment(lib, "opencv_world480d.lib")
#endif
#else
#pragma comment(lib, "ippicvmt.lib")
#ifdef NDEBUG
#pragma comment(lib, "IlmImf.lib")
#pragma comment(lib, "ippiw.lib")
#pragma comment(lib, "ittnotify.lib")
#pragma comment(lib, "libjpeg-turbo.lib")
#pragma comment(lib, "libopenjp2.lib")
#pragma comment(lib, "libpng.lib")
#pragma comment(lib, "libtiff.lib")
#pragma comment(lib, "libwebp.lib")
#pragma comment(lib, "opencv_core480.lib")
#pragma comment(lib, "opencv_highgui480.lib")
#pragma comment(lib, "opencv_imgcodecs480.lib")
#pragma comment(lib, "opencv_imgproc480.lib")
#pragma comment(lib, "zlib.lib")
#else
#pragma comment(lib, "IlmImfd.lib")
#pragma comment(lib, "ippiwd.lib")
#pragma comment(lib, "ittnotifyd.lib")
#pragma comment(lib, "libjpeg-turbod.lib")
#pragma comment(lib, "libopenjp2d.lib")
#pragma comment(lib, "libpngd.lib")
#pragma comment(lib, "libtiffd.lib")
#pragma comment(lib, "libwebpd.lib")
#pragma comment(lib, "opencv_core480d.lib")
#pragma comment(lib, "opencv_highgui480d.lib")
#pragma comment(lib, "opencv_imgcodecs480d.lib")
#pragma comment(lib, "opencv_imgproc480d.lib")
#pragma comment(lib, "zlibd.lib")
#endif
#endif	/*USE_OPENCV_WORLD_DLL*/
