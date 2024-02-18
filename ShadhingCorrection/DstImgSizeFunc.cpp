#include "stdafx.h"
#include "PhysicalSize.h"
#include "DstImgSizeFunc.h"

// [CONF] Factor to convert 1 inch to mm
const double mm_per_inch = 25.4;

namespace
{
	/// Parse string as maginification factor.
	bool parse_as_mag(const char* const str, double& mag)
	{
		char c;

		std::istringstream ist(str);
		ist >> c;
		if (!ist || c != 'x') {
			return false;
		}
		ist >> mag;
		return !!ist;
	}

	/// Parse string as PhysicalSize.
	bool parse_as_Size2d(const char* const str, PhysicalSize& psz)
	{
		std::istringstream ist(str);
		ist >> psz;
		return !!ist;
	}

}	// namespace

DstImgSizeFunc::DstImgSizeFunc()
	: m_bAsRelative(false), m_magFactor(0.0), m_dpi(0.0)
{
	/*pass*/
}

bool DstImgSizeFunc::empty() const
{
	if (m_bAsRelative) {
		return (m_magFactor <= 0.0);
	}
	else {
		return (m_ROISize.empty() || m_dpi <= 0.0);
	}
}

void DstImgSizeFunc::setMagFactor(const double magFactor)
{
	m_bAsRelative = true;
	m_magFactor = magFactor;
}

void DstImgSizeFunc::setPhysicalSize(const PhysicalSize& psz)
{
	m_bAsRelative = false;
	m_ROISize = psz;
}

void DstImgSizeFunc::setDpi(const double dpi)
{
	m_dpi = dpi;
}

double DstImgSizeFunc::getDpi() const
{
	return m_dpi;
}

int DstImgSizeFunc::isMagnificationMode() const
{
	return m_bAsRelative;
}

bool DstImgSizeFunc::getDstImgSize(const cv::Size& srcImgSize, cv::Size& dstImgSize) const
{
	if (m_bAsRelative) {
		const int widthInPx = (int)std::round(m_magFactor * srcImgSize.width);
		const int heightInPx = (int)std::round(m_magFactor * srcImgSize.height);
		dstImgSize = cv::Size(widthInPx, heightInPx);
	}
	else {
		if (m_dpi <= 0.0) {
			return false;
		}
		const int widthInPx = (int)std::round((m_dpi * m_ROISize.width()) / mm_per_inch);
		const int heightInPx = (int)std::round((m_dpi * m_ROISize.height()) / mm_per_inch);
		dstImgSize = cv::Size(widthInPx, heightInPx);
	}
	return true;
}

std::istream& DstImgSizeFunc::input(std::istream& is)
{
	std::string ROISizeStr;

	auto sv_f = is.flags();
	is.setf(std::ios::skipws);
	is >> ROISizeStr;
	is.flags(sv_f);

	if (!is) {
		return is;
	}

	PhysicalSize psz;
	double mag;
	if (parse_as_mag(ROISizeStr.c_str(), mag)) {
		// (Magnification mode)
		// Destination image size specified relatively to source image size.
		setMagFactor(mag);
	}
	else {
		// (Physical size mode)
		// Destination image size specified as (physical size * dpi).
		// Input physical size only here. The auxiliary parameter "-dpi=n" is given separately.
		if (!parse_as_Size2d(ROISizeStr.c_str(), psz)) {
			is.setstate(std::ios::failbit);
			return is;
		}
		// psz is physical size of object within ROI.
		setPhysicalSize(psz);
	}

	return is;
}

std::ostream& DstImgSizeFunc::output(std::ostream& os) const
{
	if (m_bAsRelative) {
		// (Magnification mode)
		os << "x" << m_magFactor;
	}
	else {
		// (Physical size mode)
		// Output physical size only. The auxiliary parameter "-dpi=n" is omitted.
		os << m_ROISize;
	}
	return os;
}

std::string DstImgSizeFunc::str() const
{
	std::ostringstream ost;
	output(ost);
	return ost.str();
}

std::istream& operator>>(std::istream& is, DstImgSizeFunc& dszfunc)
{
	return dszfunc.input(is);
}

std::ostream& operator<<(std::ostream& os, const DstImgSizeFunc& dszfunc)
{
	return dszfunc.output(os);
}
