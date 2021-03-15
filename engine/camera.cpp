#include <glad/glad.h>

#include <cmath>
#include <iostream>
#include <fixed_point.h>

#include "window.h"
#include "camera.h"

Camera::Camera(float _maxScale) :
widthBoundary(internalWidth),
maxScale(_maxScale),
scale(1.f),
limitRatio(FixedPoint(6)/FixedPoint(7))
{

}

Camera::Camera() : Camera(1.1f)
{

}


const FixedPoint stageWidth(480);

void Camera::Calculate(Point2d<FixedPoint> p1, Point2d<FixedPoint> p2)
{
	const FixedPoint dif = p1.x - p2.x;

	//Zooms out when the distance between points is larger than the screen's h-ratio.
	if(dif.abs() > FixedPoint(widthBoundary*limitRatio)) 
	{
		scale = dif.abs()/(widthBoundary*limitRatio);
		if(scale > maxScale)
			scale = maxScale;
	}
	else
	{
		scale = 1.f;
	}
	

	center = p1.x - dif/FixedPoint(2);
	if(GetWallPos(camera::leftWall) <= -stageWidth)
		center = -stageWidth + widthBoundary*scale/FixedPoint(2);
	else if(GetWallPos(camera::rightWall) >= stageWidth)
		center = stageWidth - widthBoundary*scale/FixedPoint(2);
}

void Camera::Apply()
{
	glTranslatef(internalWidth/2.f, 0, 0);
	glScalef(1.f/(float)scale, 1.f/(float)scale, 1);
    glTranslatef(-center, 0, 0);
}

FixedPoint Camera::GetWallPos(int which)
{
	switch(which)
	{
	case camera::leftWall:
		return center - widthBoundary*scale/FixedPoint(2);
	case camera::rightWall:
		return center + widthBoundary*scale/FixedPoint(2);
	}
	return 0;
}
