#pragma once

#include "ImgFunc_whitening02Base.h"

class ImgFunc_whitening02wwm : public ImgFunc_whitening02Base
{
protected:
	cv::Mat m_maskToAvoidFgObj;

public:
	ImgFunc_whitening02wwm(Param& param);

	const char* getName() const;
	const char* getSummary() const;

	void updateMaskToAvoidFgObj(const cv::Mat& newMaskImg);
	const cv::Mat& getMaskToAvoidFgObj() const;

private:
	/// Sample pixels.
	std::vector<LumSample> sampleImage(const cv::Mat_<uchar>& image);
};
