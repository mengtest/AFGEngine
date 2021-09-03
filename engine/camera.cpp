#include "camera.h"
#include "window.h"

#include <glad/glad.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/vec3.hpp>

#include <cmath>
#include <algorithm>
#include <iostream>
#include <fixed_point.h>

FixedPoint interpolate(FixedPoint x, FixedPoint min, FixedPoint max, FixedPoint length) {
	x = x/length;
	return (max-min)*x*x*x * (FixedPoint(3) - FixedPoint(2) * x) + min;
}

const FixedPoint stageWidth(300);
const FixedPoint stageHeight(225);

Camera::Camera(float _maxScale) :
scale(1.f),
widthBoundary(internalWidth),
heightBoundary(internalHeight),
limitRatioX(FixedPoint(6)/FixedPoint(7)),
limitRatioY(FixedPoint(2)/FixedPoint(3))
{
	if(_maxScale*(internalWidth/2.f)>(float)stageWidth)
		maxScale = (stageWidth*2.f)/(float)internalWidth;
	else
		maxScale = _maxScale;
	//No support for tall stages.
}

Camera& Camera::operator=(const Camera& c)
{
	center = c.center;
	centerTarget = c.centerTarget;
	scale = c.scale;
	scaleTimer = c.scaleTimer;
	shakeTime = c.shakeTime;
	return *this;
}

Camera::Camera() : Camera(1.1f)
{

}

void Camera::SetShakeTime(int time)
{
	if(time > shakeTime)
		shakeTime = time;
}

glm::mat4 Camera::Calculate(Point2d<FixedPoint> p1, Point2d<FixedPoint> p2)
{
	Point2d<FixedPoint> dif = p2 - p1;
	const FixedPoint distToScroll(100,0);
	//Zooms out when the distance between points is larger than the screen's h-ratio.
	
	FixedPoint targetScale = 1.f;
	FixedPoint scaleX, scaleY;
	if(dif.x.abs() > FixedPoint(widthBoundary*limitRatioX)) 
	{
		scaleX = dif.x.abs()/(widthBoundary*limitRatioX);
	}
	if(dif.y.abs() > FixedPoint(heightBoundary*limitRatioY))
	{
		scaleY = FixedPoint(0.75)+dif.y.abs()/(FixedPoint(4)*heightBoundary*limitRatioY);
	}
	auto biggerScale = std::max(scaleX, scaleY);
	if(biggerScale > targetScale)
		targetScale = biggerScale;
	if(targetScale > maxScale)
		targetScale = maxScale;
			
	if(targetScale < scale)
	{
		if(scaleTimer <= 0)
		{
			scale = interpolate(FixedPoint(scaleTimer+512),targetScale,scale,512);
		}
		--scaleTimer; 
	}
	else
	{
		scale = targetScale;
		scaleTimer = 20;
	}
	

	dif.x.value >>= 1;
	dif.y.value >>= 1;
	auto rightmost = std::max(p1.x, p2.x);
	auto leftmost = std::min(p1.x, p2.x);
	
	if(rightmost > distToScroll + centerTarget.x)
	{
		if(leftmost < -distToScroll + centerTarget.x)
			centerTarget.x.value = p1.x.value + (dif.x.value);
		else
		centerTarget.x = rightmost-distToScroll;

	}
	else if(leftmost < -distToScroll + centerTarget.x)
		centerTarget.x = leftmost+distToScroll;


	centerTarget.y = p1.y + dif.y;
	centerTarget.y -= 64;
 	if(centerTarget.y < 0)
		centerTarget.y = 0; 

	center.x += (centerTarget.x - center.x)*0.35;
	center.y += (centerTarget.y - center.y)*0.25;
	
	if(GetWallPos(camera::leftWall) <= -stageWidth)
		center.x = -stageWidth + widthBoundary*scale/FixedPoint(2);
	else if(GetWallPos(camera::rightWall) >= stageWidth)
		center.x = stageWidth - widthBoundary*scale/FixedPoint(2);
	
	centerYShake = center.y;
	if(shakeTime > 0)
	{
		centerYShake += 3*abs((shakeTime % 4) - 1);
		--shakeTime;
	}
	
	if(centerYShake > 450-(internalHeight)*(float)scale)
	{
		centerYShake = 450-(internalHeight)*(float)scale;
		centerYShake -= abs((shakeTime % 4) - 1); //Shake backwards
	}
	
	glm::mat4 view(1);

	view = glm::translate(view, glm::vec3(internalWidth/2.f, 0.f, 0.f));
	view = glm::scale(view, glm::vec3(1.f/(float)scale, 1.f/(float)scale, 1.f));
	view = glm::translate(view, glm::vec3(-center.x, -centerYShake, 0.f));
	
	return view;
}

FixedPoint Camera::GetWallPos(int which)
{
	switch(which)
	{
	case camera::leftWall:
		return center.x - widthBoundary*scale/FixedPoint(2);
	case camera::rightWall:
		return center.x + widthBoundary*scale/FixedPoint(2);
	}
	return 0;
}

centerScale Camera::GetCameraCenterScale()
{
	return {center.x, centerYShake, scale};
}