#include "fixed_point.h"

#include <cstdlib> //abs()

#ifndef FP_FRACBITS
	#define FP_FRACBITS 16
#endif

//Fractional part bits
const int fracBits = FP_FRACBITS;
const int fracUnit = 1 << fracBits;

FixedPoint::FixedPoint()
{
	value = 0;
}

FixedPoint::FixedPoint(int integral, unsigned int fractional)
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

FixedPoint& FixedPoint::operator=(FixedPoint a)
{
	value = a.value;
	return *this;
}


FixedPoint FixedPoint::operator+(FixedPoint a) const
{
	FixedPoint result;
	result.value = value + a.value;
	return result;
}


FixedPoint FixedPoint::operator-(FixedPoint a) const
{
	FixedPoint result;
	result.value = value - a.value;
	return result;
}

FixedPoint FixedPoint::operator*(FixedPoint a) const
{
	FixedPoint result;
	std::int64_t mul = static_cast<std::int64_t>(value) * static_cast<std::int64_t>(a.value);
	mul >>= fracBits;
	result.value = static_cast<fixed_t>(mul);
	return result;
}


FixedPoint FixedPoint::operator/(FixedPoint a) const
{
	FixedPoint result;
	std::int64_t div = static_cast<std::int64_t>(value) << fracBits;
	div /= a.value;
	result.value = static_cast<fixed_t>(div);
	return result;
}


void FixedPoint::operator+=(FixedPoint a)
{
	value += a.value;
}


void FixedPoint::operator-=(FixedPoint a)
{
	value -= a.value;
}

bool FixedPoint::operator<(const FixedPoint &a) const
{
	return value < a.value;
}

bool FixedPoint::operator<=(const FixedPoint &a) const
{
	return value <= a.value;
}

bool FixedPoint::operator>(const FixedPoint &a) const
{
	return value > a.value;
}

bool FixedPoint::operator>=(const FixedPoint &a) const
{
	return value >= a.value;
}

bool FixedPoint::operator==(const FixedPoint &a) const
{
	return value == a.value;
}

bool FixedPoint::operator!=(const FixedPoint &a) const
{
	return value != a.value;
}

//Unary minus
FixedPoint FixedPoint::operator-() const
{
	FixedPoint result;
	result.value = -value;
	return result;
}

//Convertion
FixedPoint::fixed_t FixedPoint::ToFixed(double number)
{
	return static_cast<fixed_t>(number*fracUnit);
}

FixedPoint::fixed_t FixedPoint::ToFixed(float number)
{
	return static_cast<fixed_t>(number*fracUnit);
}

FixedPoint::fixed_t FixedPoint::ToFixed(int number)
{
	return static_cast<fixed_t>(number << fracBits);
}

//Casting
FixedPoint::operator int() const
{
	return static_cast<int>(value >> fracBits);
}

FixedPoint::operator float() const
{
	return static_cast<float>(value) / fracUnit;
}

FixedPoint::operator double() const
{
	return static_cast<double>(value) / fracUnit;
}

//Misc
FixedPoint FixedPoint::abs() const
{
	FixedPoint fp;
	fp.value = std::abs(value);
	return fp;
}