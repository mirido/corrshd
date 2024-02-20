#include "stdafx.h"
#include "IImgFunc.h"
#include "ImgFuncBase.h"
#include "ImgFunc_whitening02.h"

#include "../libimaging/shdcutil.h"

ImgFunc_whitening02::ImgFunc_whitening02(Param& param)
	: ImgFunc_whitening02Base(param)
{
	/*pass*/
}

const char* ImgFunc_whitening02::getName() const
{
	return "whitening02";
}

const char* ImgFunc_whitening02::getSummary() const
{
	return "Shading correction with cubic polynomial regression.";
}

/// Sample pixels.
std::vector<LumSample> ImgFunc_whitening02::sampleImage(const cv::Mat_<uchar>& image)
{
	const cv::Size kernelSz = get_bin_kernel_size(image.size());
	const cv::Rect smpROI = get_bin_ROI(image.size());
	return sample_pixels(image, smpROI, kernelSz.width, kernelSz.height);
}
