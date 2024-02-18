#pragma once

char get_char_from_istream_notskipws(std::istream& is);

std::string get_str_from_istream_skipws(std::istream& is);

std::string get_line_from_istream(std::istream& is);

void toupper_str(std::string& str);

void tolower_str(std::string& str);

bool get_selection_from_istream(
	std::ostream& os,
	std::istream& is,
	const char* const selectionItemStr,
	std::string& result,
	const bool bEnableDefault
);
