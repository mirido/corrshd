#pragma once

class WithStdWhiteImg
{
protected:
	cv::Mat m_stdWhiteImg;
	bool m_bNeedStdWhiteImg;

public:
	WithStdWhiteImg();

	void needStdWhiteImg(const bool bNeed);

protected:
	void updateStdWhiteImg(const cv::Mat& newImg);

public:
	const cv::Mat getStdWhiteImg() const;

};
