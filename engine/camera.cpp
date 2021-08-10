#include "camera.h"
#include "window.h"

#include <glad/glad.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/vec3.hpp>

#include <cmath>
#include <iostream>
#include <fixed_point.h>

const FixedPoint stageWidth(300);

Camera::Camera(float _maxScale) :
scale(1.f),
widthBoundary(internalWidth),
limitRatio(FixedPoint(6)/FixedPoint(7)),
maxScale(_maxScale)
{
	if(maxScale*(internalWidth/2)>stageWidth)
		maxScale = (stageWidth*2)/internalWidth;
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

	//Zooms out when the distance between points is larger than the screen's h-ratio.
	if(dif.x.abs() > FixedPoint(widthBoundary*limitRatio)) 
	{
		scale = dif.x.abs()/(widthBoundary*limitRatio);
		if(scale > maxScale)
			scale = maxScale;
		//std::cout <<(float)(scale*internalWidth)<<"\n";
	}
	else
	{
		scale = 1.f;
	}
	
	dif.x.value >>= 1;
	dif.y.value >>= 1;



	center = p1 + dif;
	center.y -= 64;

 	if(center.y < 0)
		center.y = 0; 
	if(GetWallPos(camera::leftWall) <= -stageWidth)
		center.x = -stageWidth + widthBoundary*scale/FixedPoint(2);
	else if(GetWallPos(camera::rightWall) >= stageWidth)
		center.x = stageWidth - widthBoundary*scale/FixedPoint(2);

	if(shakeTime > 0)
	{
		center.y.value += shakeTime*((shakeTime%2)<<10);
		--shakeTime;
	}

	if(center.y > 450-internalHeight)
	{
		center.y = 450-internalHeight;
		center.y.value -= shakeTime*((shakeTime%2)<<10);
	}
	
	glm::mat4 view(1);

	view = glm::translate(view, glm::vec3(internalWidth/2.f, 0.f, 0.f));
	view = glm::scale(view, glm::vec3(1.f/(float)scale, 1.f/(float)scale, 1.f));
	view = glm::translate(view, glm::vec3(-center.x, -center.y, 0.f));
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
