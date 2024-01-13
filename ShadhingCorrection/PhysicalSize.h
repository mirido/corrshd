#pragma once

class PhysicalSize
{
	double m_width;
	double m_height;

public:
	PhysicalSize();
	double width() const;
	double height() const;
	std::istream& input(std::istream& is);

};

std::istream& operator>>(std::istream& is, PhysicalSize& psize);
