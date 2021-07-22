#ifndef POINT_H_INCLUDED
#define POINT_H_INCLUDED

#include <utility>


template <class T>
class Point2d
{
public:
	T x, y;
	
public:
	Point2d();
	Point2d(const Point2d& p);
	Point2d(T _x, T _y);

	Point2d& operator=(Point2d a);
	Point2d operator+(Point2d a);
	Point2d operator-(Point2d a);
	Point2d& operator+=(Point2d a);
	
};

template <class T>
Point2d<T>::Point2d()
{

}

template <class T>
Point2d<T>::Point2d(const Point2d<T>& p) : Point2d()
{
	x = p.x;
	y = p.y;
}

template <class T>
Point2d<T>::Point2d(T _x, T _y): Point2d()
{
	x = _x;
	y = _y;
}

template <class T>
Point2d<T>& Point2d<T>::operator=(Point2d<T> a)
{
	x = a.x;
	y = a.y;
	return *this;
}

template <class T>
Point2d<T> Point2d<T>::operator+(Point2d<T> op)
{
	Point2d<T> result;
	result.x = x + op.x;
	result.y = y + op.y;
	return result;
}

template <class T>
Point2d<T> Point2d<T>::operator-(Point2d<T> op)
{
	Point2d<T> result;
	result.x = x - op.x;
	result.y = y - op.y;
	return result;
}

template <class T>
Point2d<T>& Point2d<T>::operator+=(Point2d<T> a)
{
	x += a.x;
	y += a.y;
	return *this;
}


#endif //POINT_H_INCLUDED