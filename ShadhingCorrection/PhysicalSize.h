#pragma once

class PhysicalSize
{
	double m_width;
	double m_height;

public:
	PhysicalSize();
	bool empty() const;
	double width() const;
	double height() const;
	std::istream& input(std::istream& is);
	std::ostream& output(std::ostream& os) const;

	std::string str() const;

};

std::istream& operator>>(std::istream& is, PhysicalSize& psize);
std::ostream& operator<<(std::ostream& os, const PhysicalSize& psize);
