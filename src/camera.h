#ifndef CAMERA_H_INCLUDED
#define CAMERA_H_INCLUDED

#include <fixed_point.h>
#include <geometry.h>

namespace camera
{
	enum
	{
		leftWall,
		rightWall,
	};
}

class Camera
{
public: //As always, this is access only.
	FixedPoint center;

private:

	const FixedPoint widthBoundary;
	const FixedPoint limitRatio; //The max distance relative to 1 game screen between characters before the camera zooms out.
	const FixedPoint maxScale; //Max zoom out.
	FixedPoint scale;

public:
    Camera(float maxScale);

    void Apply();
	void Calculate(Point2d<FixedPoint>, Point2d<FixedPoint> p2);

    FixedPoint GetWallPos(int which);
private:
};


#endif // CAMERA_H_INCLUDED
