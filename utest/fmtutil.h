#pragma once

//
//	FormatSpec
//

struct FormatSpec
{
	std::locale m_locale;
	std::ios::fmtflags m_fmtflags;
	std::streamsize m_precision;
	char m_fillChar;

	FormatSpec();
	void saveAndChange(
		const std::ios::fmtflags fmtFlags,
		const std::streamsize precision,
		const char fillChar,
		std::ostream& ost
	);
	void restore(std::ostream& ost) const;
};

//
//	Save and change interface
//

/// Save and change real number format of ost.
void save_and_change_real_num_format(
	std::ostream& ost,
	FormatSpec& sv_spec,
	const bool bFixed,
	const int precision
);

/// Save and change fill char of ost.
void save_and_change_fillchar(
	std::ostream& ost,
	FormatSpec& sv_spec,
	const char fillChar
);

/// Restore format of ost.
void restore_format(const FormatSpec& sv_spec, std::ostream& ost);

//
//	Change interface
//

/// Change real number format of ost.
void change_real_num_format(
	std::ostream& ost,
	const bool bFixed,
	const int precision
);

/// Change fill char of ost.
void change_fillchar_after_saving(
	std::ostream& ost,
	const char fillChar
);
