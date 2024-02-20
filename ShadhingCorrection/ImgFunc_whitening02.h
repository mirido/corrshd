#pragma once

#include "ImgFunc_whitening02Base.h"

class ImgFunc_whitening02 : public ImgFunc_whitening02Base
{
public:
	ImgFunc_whitening02(Param& param);

	const char* getName() const;
	const char* getSummary() const;

private:
	/// Sample pixels.
	std::vector<LumSample> sampleImage(const cv::Mat_<uchar>& image);

};
