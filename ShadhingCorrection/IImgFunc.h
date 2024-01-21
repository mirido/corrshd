#pragma once

class IImgFunc
{
public:
	virtual ~IImgFunc() { }

	/// Get algorithm name.
	virtual const char* getName() const = 0;

	/// Run image processing.
	virtual bool run(const cv::Mat& SrcImg, cv::Mat& dstImg) = 0;

	//
	//	For DEBUG
	//

	/// Reset image dump count.
	virtual void resetImgDumpCnt() = 0;

	/// Dump intermediate image.
	virtual void dumpImg(const cv::Mat& image, const char* const caption, const char* const dstDir) = 0;

};
