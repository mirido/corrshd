#include "stdafx.h"
#include "fmtutil.h"

//
//	FormatSpec
//

FormatSpec::FormatSpec()
	: m_fmtflags(0), m_precision(std::streamsize(6)), m_fillChar(' ')
{
	/*pass*/
}

void FormatSpec::saveAndChange(
	const std::ios::fmtflags fmtFlags,
	const std::streamsize precision,
	const char fillChar,
	std::ostream& ost
)
{
	// Set locale to "C".
	m_locale = ost.imbue(std::locale("C", std::locale::all));

	// Set fixed format or not.
	m_fmtflags = ost.flags(fmtFlags);

	// Set prrecision.
	m_precision = ost.precision(precision);

	// Set fill char.
	m_fillChar = ost.fill(fillChar);
}

void FormatSpec::restore(std::ostream& ost) const
{
	ost.imbue(m_locale);
	ost.flags(m_fmtflags);
	ost.precision(m_precision);
	ost.fill(m_fillChar);
}

//
//	Save and change interface
//

/// Save and change real number format of ost.
void save_and_change_real_num_format(
	std::ostream& ost,
	FormatSpec& sv_spec,
	const bool bFixed,
	const int precision_
)
{
	auto fmtflags = ost.flags();
	if (bFixed) {
		fmtflags |= std::ios::fixed;
	}
	else {
		fmtflags &= ~std::ios::fixed;
	}
	auto precision = std::streamsize(precision_);
	const char fillChar = ost.fill();
	sv_spec.saveAndChange(fmtflags, precision, fillChar, ost);
}

/// Save and change fill char of ost.
void save_and_change_fillchar(
	std::ostream& ost,
	FormatSpec& sv_spec,
	const char fillChar
)
{
	auto fmtflags = ost.flags();
	auto precision = ost.precision();
	sv_spec.saveAndChange(fmtflags, precision, fillChar, ost);
}

/// Restore format of ost.
void restore_format(const FormatSpec& sv_spec, std::ostream& ost)
{
	sv_spec.restore(ost);
}

//
//	Change interface
//

/// Change real number format of ost.
void change_real_num_format(
	std::ostream& ost,
	const bool bFixed,
	const int precision
)
{
	FormatSpec dummy;
	save_and_change_real_num_format(ost, dummy, bFixed, precision);
}

/// Change fill char of ost.
void change_fillchar_after_saving(
	std::ostream& ost,
	const char fillChar
)
{
	FormatSpec dummy;
	save_and_change_fillchar(ost, dummy, fillChar);
}
