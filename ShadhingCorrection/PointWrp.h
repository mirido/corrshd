#pragma once

struct PointWrp : public cv::Point
{
	PointWrp();
	PointWrp(int x, int y);
	explicit PointWrp(const cv::Point& obj);

	std::istream& input(std::istream& is);
	std::ostream& output(std::ostream& os) const;
	std::string str() const;
};

std::istream& operator>>(std::istream& is, PointWrp& pt);
std::ostream& operator<<(std::ostream& os, const PointWrp& pt);
