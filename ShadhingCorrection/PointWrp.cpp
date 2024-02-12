#include "stdafx.h"
#include "PointWrp.h"

PointWrp::PointWrp()
{
	/*pass*/
}

PointWrp::PointWrp(int x, int y)
	: cv::Point(x, y)
{
	/*pass*/
}

PointWrp::PointWrp(const cv::Point& obj)
	: cv::Point(obj)
{
	/*pass*/
}

std::istream& PointWrp::input(std::istream& is)
{
	int tmp;
	char c;

	x = y = 0;

	auto sv_f = is.flags();
	is.setf(std::ios::skipws);

	// Input example: "(429, 3955)"

	// Feed "(".
	is >> c;
	if (!is) {
		goto exit_immediately;
	}
	if (c != '(') {
		is.setstate(std::ios::failbit);
		goto exit_immediately;
	}

	// Read x.
	is >> tmp;
	if (!is) {
		goto exit_immediately;
	}
	x = tmp;

	// Feed ",".
	is >> c;
	if (!is) {
		goto exit_immediately;
	}
	if (c != ',') {
		is.setstate(std::ios::failbit);
		goto exit_immediately;
	}

	// Read y.
	is >> tmp;
	if (!is) {
		goto exit_immediately;
	}
	y = tmp;

	// Feed ")".
	is >> c;
	if (!is) {
		goto exit_immediately;
	}
	if (c != ')') {
		is.setstate(std::ios::failbit);
		goto exit_immediately;
	}

exit_immediately:
	is.flags(sv_f);
	return is;
}

std::ostream& PointWrp::output(std::ostream& os) const
{
	os << "(" << x << "," << y << ")";
	return os;
}

std::string PointWrp::str() const
{
	std::ostringstream ost;
	output(ost);
	return ost.str();
}

std::istream& operator>>(std::istream& is, PointWrp& pt)
{
	return pt.input(is);
}

std::ostream& operator<<(std::ostream& os, const PointWrp& pt)
{
	return pt.output(os);
}
