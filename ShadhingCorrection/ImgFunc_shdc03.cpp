#include "stdafx.h"
#include "IImgFunc.h"
#include "ImgFuncBase.h"
#include "ImgFunc_whitening02.h"
#include "ImgFunc_uniform.h"
#include "ImgFunc_shdc03.h"

#include "../libnumeric/numericutil.h"
#include "../libimaging/geometryutil.h"
#include "../libimaging/shdcutil.h"

ImgFunc_shdc03::ImgFunc_shdc03()
{
	m_imgFunc_Whiteing.needMaskNearZeroToZero(true);
	m_imgFunc_Whiteing.doFinalInversion(false);
}

const char* ImgFunc_shdc03::getName() const
{
	return "shd03";
}

const char* ImgFunc_shdc03::getSummary() const
{
	return "Corrects lighting tilt by cubic regression.";
}

bool ImgFunc_shdc03::run(const cv::Mat& srcImg, cv::Mat& dstImg)
{
	// Whitening.
	cv::Mat invWhitenedImage;
	m_imgFunc_Whiteing.run(srcImg, invWhitenedImage);

	dumpImg(m_imgFunc_Whiteing.getMaskNearZeroToZero(), "mask near 0 to 0", DBG_IMG_DIR);

	const cv::Mat kernel = get_bin_kernel(srcImg.size());

	// Eliminate bakcground noize.
#if 0
#if 1
	cv::Mat maskForCleanupBG;
	cv::dilate(invWhitenedImage, maskForCleanupBG, kernel);
	const double th2 = cv::threshold(invWhitenedImage, maskForCleanupBG, 0, 255, cv::THRESH_OTSU);
	cout << "th2=" << th2 << endl;
	cv::dilate(maskForCleanupBG, maskForCleanupBG, kernel);
	dumpImg(maskForCleanupBG, "mask for cleanup background", DBG_IMG_DIR);
	cv::bitwise_and(invWhitenedImage, maskForCleanupBG, invWhitenedImage);
#else
	cv::Mat tmp;
	const double th2 = cv::threshold(invWhitenedImage, tmp, 0, 255, cv::THRESH_OTSU);
	cout << "th2=" << th2 << endl;
	tmp.release();
	cv::threshold(invWhitenedImage, invWhitenedImage, th2 * 0.5, 255.0, cv::THRESH_TOZERO);
#endif
	dumpImg(invWhitenedImage, "cleanup image", DBG_IMG_DIR);
#endif

	// Uniform.
	cv::Mat invUniformedImage;
	m_imgFunc_uniform.run(invWhitenedImage, invUniformedImage);

	cv::bitwise_not(invUniformedImage, dstImg);

	return true;
}
