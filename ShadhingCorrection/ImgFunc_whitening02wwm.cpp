#include "stdafx.h"
#include "IImgFunc.h"
#include "ImgFuncBase.h"
#include "ImgFunc_whitening02wwm.h"

#include "../libimaging/shdcutil.h"

ImgFunc_whitening02wwm::ImgFunc_whitening02wwm(Param& param)
	: ImgFunc_whitening02Base(param)
{
	/*pass*/
}

const char* ImgFunc_whitening02wwm::getName() const
{
	return "whitening02wwm";
}

const char* ImgFunc_whitening02wwm::getSummary() const
{
	return "Whitening02 based on whole image with mask.";
}

void ImgFunc_whitening02wwm::updateMaskToAvoidFgObj(const cv::Mat& newMaskImg)
{
	m_maskToAvoidFgObj = newMaskImg.clone();
}

const cv::Mat& ImgFunc_whitening02wwm::getMaskToAvoidFgObj() const
{
	return m_maskToAvoidFgObj;
}

/// Sample pixels.
std::vector<LumSample> ImgFunc_whitening02wwm::sampleImage(const cv::Mat_<uchar>& image)
{
	// Sample from whole image.
	const cv::Size kernelSz = get_bin_kernel_size(image.size());
	const cv::Rect smpROI = cv::Rect(0, 0, image.cols, image.rows);		// Whole image. Not 80% etc.
	const std::vector<LumSample> ptsFull = sample_pixels(image, smpROI, kernelSz.width, kernelSz.height);

	// Exclude outside the mask.
	std::vector<LumSample> pts;
	pts.reserve(ptsFull.size());
	for (auto it = ptsFull.begin(); it != ptsFull.end(); it++) {
		if (m_maskToAvoidFgObj.at<uchar>(it->m_point) != C_UCHAR(0)) {
			pts.push_back(*it);
		}
	}
	pts.resize(pts.size());
	return pts;
}
