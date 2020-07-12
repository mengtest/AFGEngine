#ifndef FIXEDPOINT_H_INCLUDED
#define FIXEDPOINT_H_INCLUDED

#include <cstdint> //fixed width types

//Fixed point, 32bit as 16.16.
class FixedPoint
{
public:
	typedef std::int32_t fixed_t;
	fixed_t value;

public:
	FixedPoint();
	FixedPoint(int decimal, unsigned int fractional);



	FixedPoint& operator=(FixedPoint a);

	FixedPoint operator+(FixedPoint a) const;
	FixedPoint operator-(FixedPoint a) const;
	void operator+=(FixedPoint a);
	void operator-=(FixedPoint a);

	FixedPoint operator/(FixedPoint a) const;
	FixedPoint operator*(FixedPoint a) const;

	bool operator<(const FixedPoint &a) const;
	bool operator<=(const FixedPoint &a) const;
	bool operator>(const FixedPoint &a) const;
	bool operator>=(const FixedPoint &a) const;
	bool operator==(const FixedPoint &a) const;
	bool operator!=(const FixedPoint &a) const;

	FixedPoint operator-() const;

	operator int() const;
	operator float() const;
	operator double() const;

	FixedPoint abs() const;

	template<typename T> FixedPoint(T number);

	template<typename T> FixedPoint& operator=(T a);

	template<typename T> FixedPoint operator+(T a) const;
	template<typename T> FixedPoint operator-(T a) const;
	template<typename T> void operator+=(T a);
	template<typename T> void operator-=(T a);

	template<typename T> bool operator<(const T &a) const;
	template<typename T> bool operator<=(const T &a) const;
	template<typename T> bool operator>(const T &a) const;
	template<typename T> bool operator>=(const T &a) const;
	template<typename T> bool operator==(const T &a) const;
	template<typename T> bool operator!=(const T &a) const;


	template<typename T> FixedPoint operator/(T a) const;
	template<typename T> FixedPoint operator*(T a) const;

private:
	static fixed_t ToFixed(double number);
	static fixed_t ToFixed(float number);
	static fixed_t ToFixed(int number);
};

template<typename T>
FixedPoint::FixedPoint(T number)
{
	value = ToFixed(number);
}

template<typename T>
FixedPoint& FixedPoint::operator=(T a)
{
	value = ToFixed(a);
	return *this;
}

//Arithmetic

template<typename T>
FixedPoint FixedPoint::operator+(T a) const
{
	const FixedPoint operand(a);
	return operator+(operand);
}

template<typename T>
FixedPoint FixedPoint::operator-(T a) const
{
	const FixedPoint operand(a);
	return operator-(operand);
}

template<typename T>
FixedPoint FixedPoint::operator*(T a) const
{
	const FixedPoint operand(a);
	return operator*(operand);
}

template<typename T>
FixedPoint FixedPoint::operator/(T a) const
{
	const FixedPoint operand(a);
	return operator/(operand);
}

template<typename T>
void FixedPoint::operator+=(T a)
{
	const FixedPoint operand(a);
	*this = (*this)+operand;
}

template<typename T>
void FixedPoint::operator-=(T a)
{
	const FixedPoint operand(a);
	*this = (*this)-operand;
}

//Boolean operator

template<typename T>
bool FixedPoint::operator<(const T &a) const
{
	return *this < FixedPoint(a);
}

template<typename T>
bool FixedPoint::operator<=(const T &a) const
{
	return *this <= FixedPoint(a);
}

template<typename T>
bool FixedPoint::operator>(const T &a) const
{
	return *this > FixedPoint(a);
}

template<typename T>
bool FixedPoint::operator>=(const T &a) const
{
	return *this >= FixedPoint(a);
}

template<typename T>
bool FixedPoint::operator==(const T &a) const
{
	return *this == FixedPoint(a);
}

template<typename T>
bool FixedPoint::operator!=(const T &a) const
{
	return *this != FixedPoint(a);
}

#endif //FIXEDPOINT_H_INCLUDED
