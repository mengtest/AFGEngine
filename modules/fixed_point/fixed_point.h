#ifndef FIXEDPOINT_H_INCLUDED
#define FIXEDPOINT_H_INCLUDED

#include <cstdint> //fixed width types

#ifndef FP_FRACBITS
	#define FP_FRACBITS 16
#endif

	
//Fixed point, 32bit as 16.16.
class FixedPoint
{
	
public:
	typedef std::int32_t fixed_t;
	fixed_t value;

public:
	constexpr FixedPoint();
	constexpr FixedPoint(int decimal, unsigned int fractional);



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
	static constexpr int fracBits = FP_FRACBITS;
	static constexpr int fracUnit = 1 << fracBits;

	static fixed_t ToFixed(double number);
	static fixed_t ToFixed(float number);
	static fixed_t ToFixed(int number);
};


constexpr FixedPoint::FixedPoint() : value(0)
{
	
}

constexpr FixedPoint::FixedPoint(int integral, unsigned int fractional) : value(0)
{

	bool negate = false;
	if(integral < 0)
	{
		integral = -integral;
		negate = true;
	}
	
	//Set the integral part in the msb
	integral <<= fracBits;

	//Make sure the fractional part fits in the lsb
	int c = 0; //Compensation
	while(fractional >= fracUnit)
	{	
		fractional >>= 1;
		c++;
	}

	//Base 10 fractional to base 2
	fractional <<= fracBits;
	while(fractional >= 1 << (fracBits))
	{	
		fractional /= 10;
		if(c > 0)
		{
			fractional <<= 1;
			c--;
		}
	}

	value = integral + fractional;
	if(negate)
		value = -value;
}

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
