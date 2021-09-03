#ifndef CAMERA_H_INCLUDED
#define CAMERA_H_INCLUDED

#include <fixed_point.h>
#include <geometry.h>
#include <glm/mat4x4.hpp>

namespace camera
{
	enum
	{
		leftWall,
		rightWall,
	};
}

struct centerScale
{
	float x, y, scale;
};

class Camera
{
public: //As always, this is access only.
	Point2d<FixedPoint> center;
	Point2d<FixedPoint> centerTarget;
	FixedPoint scale;

private:
	const FixedPoint widthBoundary;
	const FixedPoint heightBoundary;
	const FixedPoint limitRatioX; //The max distance relative to 1 game screen between characters before the camera zooms out.
	const FixedPoint limitRatioY;
	FixedPoint maxScale; //Max zoom out.
	float centerYShake;
	int scaleTimer = 0;
	int shakeTime = 0;

public:
	Camera();
	Camera(float maxScale);
	Camera& operator=(const Camera& c);

	glm::mat4 Calculate(Point2d<FixedPoint>, Point2d<FixedPoint> p2);
	centerScale GetCameraCenterScale();
	

	FixedPoint GetWallPos(int which);
	void SetShakeTime(int time);
private:
};


#endif // CAMERA_H_INCLUDED
