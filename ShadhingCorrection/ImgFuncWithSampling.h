#pragma once

#include "../libimaging/imaging_op.h"	/*introduce LumSample*/

class ImgFuncWithSampling : public ImgFuncBase
{
public:
	ImgFuncWithSampling(ParamPtr pParam);

	//
	//	For DEBUG
	//

	/// Dump approximation result visually. (For DEBUG.)
	void dumpAppxImg(
		const cv::Mat srcImg,
		const std::vector<double>& cflist,
		const char* const caption
	);

	/// Plot sample points. (For DEBUG.)
	void plotSamples(
		const cv::Mat_<uchar>& srcImg,
		const std::vector<LumSample>& samples,
		const char* const caption
	);

};
