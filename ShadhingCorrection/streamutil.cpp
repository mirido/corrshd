#include "stdafx.h"
#include "streamutil.h"

char get_char_from_istream_notskipws(std::istream& is)
{
	char cTmp;

	const auto sv_f = is.flags();
	is.unsetf(std::ios::skipws);
	is >> cTmp;
	is.flags(sv_f);

	return cTmp;
}

std::string get_str_from_istream_skipws(std::istream& is)
{
	std::string strTmp;

	const auto sv_f = is.flags();
	is.setf(std::ios::skipws);
	is >> strTmp;
	is.flags(sv_f);

	return strTmp;
}

std::string get_line_from_istream(std::istream& is)
{
	std::string strTmp;
	std::getline(is, strTmp);
	return strTmp;
}

void toupper_str(std::string& str)
{
#pragma warning(push)
#pragma warning(disable : 4244)
	std::transform(str.begin(), str.end(), str.begin(), ::toupper);
#pragma warning(pop)
}

void tolower_str(std::string& str)
{
#pragma warning(push)
#pragma warning(disable : 4244)
	std::transform(str.begin(), str.end(), str.begin(), ::tolower);
#pragma warning(pop)
}

bool get_selection_from_istream(
	std::ostream& os,
	std::istream& is,
	const char* const selectionItemStr,
	std::string& result,
	const bool bEnableDefault
)
{
	std::string strTmp;

	result.clear();

	// Make selection item list.
	std::vector<std::string> items;
	std::string promptStr;
	{
		std::istringstream ist(selectionItemStr);
		while (!ist.eof()) {
			strTmp = get_str_from_istream_skipws(ist);
			if (!ist) {
				throw std::logic_error("*** ERR ***");
			}

			// Update prompt string.
			if (!promptStr.empty()) {
				promptStr += " / ";
			}
			promptStr += strTmp;

			// Update selection item list.
			toupper_str(strTmp);
			items.push_back(strTmp);
		}
	}

	// Get input.
	if (bEnableDefault) {
		os << "(" << promptStr << ")[" << items.front() << "]: ";
	}
	else {
		os << "(" << promptStr << "): ";
	}
	std::string inputStr = get_line_from_istream(is);
	if (!is) {
		return false;
	}
	if (bEnableDefault) {
		if (inputStr.empty()) {
			result = items.front();
			return true;
		}
	}
	toupper_str(inputStr);

	// Match input.
	for (auto it = items.begin(); it != items.end(); it++) {
		const size_t pos = it->find(inputStr);
		if (pos == ZT(0)) {
			if (result.empty()) {
				result = *it;
				toupper_str(result);
			}
			else {
				// (Selection ambiguous.)
				throw std::logic_error("*** ERR ***");
			}
		}
	}

	return true;
}
