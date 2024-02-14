#pragma once

/// Represents the function that gives destination image size.
/// Destination image size is determined when method getDstImgSize is called.
class DstImgSizeFunc
{
	bool m_bAsRelative;

	// Mode 1: Destination image size specified relatively to source image size.
	double m_magFactor;				// Magnification factor relative to source image.

	// Mode 2: Destination image size specified as (physical size * dpi).
	PhysicalSize m_ROISize;			// ROI physical size in mm (or standart paper size name)
	double m_dpi;					// Resolution in dpi

public:
	DstImgSizeFunc();
private:
	void setMagFactor(const double magFactor);
	void setPhysicalSize(const PhysicalSize& psz);
public:
	void setDpi(const double dpi);
	double getDpi() const;
	int isMagnificationMode() const;
	bool getDstImgSize(const cv::Size& srcImgSize, cv::Size& dstImgSize) const;

	std::istream& input(std::istream& is);
	std::ostream& output(std::ostream& os) const;
	std::string str() const;
};

std::istream& operator>>(std::istream& is, DstImgSizeFunc& dszfunc);
std::ostream& operator<<(std::ostream& os, const DstImgSizeFunc& dszfunc);
