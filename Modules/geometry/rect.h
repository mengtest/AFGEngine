#ifndef RECT_H_INCLUDED
#define RECT_H_INCLUDED

#include "point.h"
#include <utility> //swap

#include <iostream>

template <class T>
class Rect2d
{
public:
	Point2d<T> bottomLeft, topRight;
	
public:
	Rect2d();
	Rect2d(Point2d<T> point1, Point2d<T> point2);
	Rect2d(T point1x, T point1y, T point2x, T point2y);

	Rect2d& operator=(Rect2d a);

	Rect2d Translate(Point2d<T> translate);
	Rect2d FlipHorizontal();

	bool Intersects(Rect2d rect);
	
	void printCoords();
private:
	void Normalize();
};

template <class T>
Rect2d<T>::Rect2d()
{

}

template <class T>
Rect2d<T>::Rect2d(Point2d<T> point1, Point2d<T> point2):
bottomLeft(point1),
topRight(point2)
{
	Normalize();
}

template <class T>
Rect2d<T>::Rect2d(T point1x, T point1y, T point2x, T point2y):
bottomLeft(point1x, point1y),
topRight(point2x, point2y)
{
	Normalize();
}

template <class T>
Rect2d<T>& Rect2d<T>::operator=(Rect2d<T> a)
{
	bottomLeft = a.bottomLeft;
	topRight = a.topRight;
	return *this;
}

template <class T>
Rect2d<T> Rect2d<T>::Translate(Point2d<T> amount)
{
	Rect2d translated(bottomLeft+amount, topRight+amount);
	//translated.Normalize();
	return translated;
}

template <class T>
Rect2d<T> Rect2d<T>::FlipHorizontal()
{
	Rect2d flipped(-bottomLeft.x, bottomLeft.y, -topRight.x, topRight.y);
	return flipped;
}

template <class T>
void Rect2d<T>::Normalize()
{
	//X
	if(bottomLeft.x > topRight.x)
	{
		std::swap<T>(bottomLeft.x, topRight.x);
	}
	//Y
	if(bottomLeft.y > topRight.y)
	{
		std::swap<T>(bottomLeft.y, topRight.y);
	}
}

template <class T>
bool Rect2d<T>::Intersects(Rect2d<T> b)
{
	Rect2d<T>& a = *this;

	//return whether a and b are NOT separate.
	return !( 
		a.topRight.x <= b.bottomLeft.x ||
		b.topRight.x <= a.bottomLeft.x ||
		a.topRight.y <= b.bottomLeft.y ||
		b.topRight.y <= a.bottomLeft.y
	);
}

template <class T>
void Rect2d<T>::printCoords()
{
	std::cout <<
		"Bottom left: (" << (float)bottomLeft.x << ", " << (float)bottomLeft.y << ") " << 
		"Top Right: (" << (float)topRight.x << ", " << (float)topRight.y << ") \n"; 
}


#endif //RECT_H_INCLUDED