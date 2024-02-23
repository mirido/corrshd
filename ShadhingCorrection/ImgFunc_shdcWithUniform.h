#pragma once

class ImgFunc_shdcWithUniform : public ImgFuncBase
{
	ImgFunc_uniform m_uniform;

public:
	ImgFunc_shdcWithUniform(Param& param);

	bool run(const cv::Mat& SrcImg, cv::Mat& dstImg);

private:
	virtual bool whitening(
		const cv::Mat& srcImg,
		cv::Mat& dstImg,
		cv::Mat& maskToKeepDrawLine
	) = 0;

};
