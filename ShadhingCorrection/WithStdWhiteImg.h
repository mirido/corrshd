#pragma once

class WithStdWhiteImg
{
	cv::Mat m_stdWhiteImg;
	bool m_bMakeStdWhiteImg;

public:
	WithStdWhiteImg();

	void setFlagToMakeStdWhiteImg(const bool bMake);

	bool getFlagToMakeStdWhiteImg() const;

protected:
	void updateStdWhiteImg(const cv::Mat& newImg);

	void releaseStdWhiteImg();

public:
	const cv::Mat& getStdWhiteImg() const;

};
