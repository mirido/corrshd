#include "stdafx.h"
#include "PhysicalSize.h"

#include "../libnumeric/numericutil.h"

namespace
{
	// Definition of standard paper sizes
	const std::pair<const char*, const char*> stdPaperSizeList[] = {
		{ "A0", "841x1189" },
		{ "A1", "594x841" },
		{ "A2", "420x594" },
		{ "A3", "297x420" },
		{ "A4", "210x297" },
		{ "A5", "148x210" },
		{ "A6", "105x148" },
		{ "A7", "74x105" },
		{ "A8", "52x74" },
		{ "A9", "37x52" },
		{ "A10", "26x37" },
		{ "B0", "1030x1456" },
		{ "B1", "728x1030" },
		{ "B2", "515x728" },
		{ "B3", "364x515" },
		{ "B4", "257x364" },
		{ "B5", "182x257" },
		{ "B6", "128x182" },
		{ "B7", "91x128" },
		{ "B8", "64x91" },
		{ "B9", "45x64" },
		{ "B10", "32x45" },
	};

	bool parse_size_str(const char* size_str, double& width, double& height)
	{
		double tmp;
		char c;

		width = height = 0.0;

		std::istringstream ist(size_str);
		ist.setf(std::ios::skipws);

		// Read width from str_size
		ist >> tmp;
		if (!ist) {
			return false;
		}
		width = tmp;

		// Skip 'x' in size_str
		ist >> c;
		if (c != 'x') {
			return false;
		}

		// Read height from size_str
		ist >> tmp;
		if (!ist) {
			return false;
		}
		height = tmp;

		// Check tail of size_str
		if (!ist.eof()) {
			return false;
		}

		return true;
	}

	std::map<std::string, cv::Size2d> gen_std_paper_size_dic()
	{
		std::map<std::string, cv::Size2d> map;
		const size_t sz = sizeof(stdPaperSizeList) / sizeof(stdPaperSizeList[0]);
		for (size_t i = 0; i < sz; i++) {
			const char* const name = stdPaperSizeList[i].first;
			const char* const size_str = stdPaperSizeList[i].second;

			double width, height;
			if (!parse_size_str(size_str, width, height)) {
				// (The size_str has syntax error)
				throw std::logic_error("*** ERR ***");
			}

			// Register portrait size
			{
				const cv::Size2d size(width, height);
				auto res = map.insert(std::make_pair(name, size));
				if (!res.second) {
					// (The name is duplicated)
					throw std::logic_error("*** ERR ***");
				}
			}

			// Register landscape size
			{
				std::string lnsName = std::string("L") + std::string(name);
				const cv::Size2d lnsSize(height, width);
				auto res = map.insert(std::make_pair(lnsName, lnsSize));
				if (!res.second) {
					// (The name is duplicated)
					throw std::logic_error("*** ERR ***");
				}
			}
		}
		return map;
	}

	// Dictionary of standard paper sizes
	const std::map<std::string, cv::Size2d> stdPaperSizeDic = gen_std_paper_size_dic();

}	// namespace

PhysicalSize::PhysicalSize()
	: m_width(0.0), m_height(0.0)
{
	/*pass*/
}

double PhysicalSize::width() const
{
	return m_width;
}

double PhysicalSize::height() const
{
	return m_height;
}

std::istream& PhysicalSize::input(std::istream& is)
{
	std::string buf;

	auto sv_f = is.flags();
	is.setf(std::ios::skipws);
	is >> buf;
	is.flags(sv_f);

	auto found_it = stdPaperSizeDic.find(buf);
	if (found_it != stdPaperSizeDic.end()) {
		// (Standard paper size)
		decltype(auto) sz = found_it->second;
		m_width = sz.width;
		m_height = sz.height;
	}
	else {
		// (Specific width and height value in mm)
		if (!parse_size_str(buf.c_str(), m_width, m_height)) {
			// (Syntax error)
			is.setstate(std::ios::failbit);
			return is;
		}
	}

	return is;
}

std::ostream& PhysicalSize::output(std::ostream& os) const
{
	// Return as standard paper size name whenever possible.
	for (auto it = stdPaperSizeDic.begin(); it != stdPaperSizeDic.end(); it++) {
		const cv::Size2d& s = it->second;		// Alias
		if (can_equal(s.width, m_width) && s.height == m_height) {
			os << it->first;
			return os;
		}
	}

	os << m_width << "x" << m_height;
	return os;
}

std::string PhysicalSize::str() const
{
	std::ostringstream ost;
	output(ost);
	return ost.str();
}

std::istream& operator>>(std::istream& is, PhysicalSize& psize)
{
	return psize.input(is);
}

std::ostream& operator<<(std::ostream& os, const PhysicalSize& psize)
{
	return psize.output(os);
}
