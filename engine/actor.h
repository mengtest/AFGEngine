#ifndef ACTOR_H_GUARD
#define ACTOR_H_GUARD

#include "framedata.h"
#include <geometry.h>
#include <fixed_point.h>
#include <sol/sol.hpp>

class Actor{
	sol::state lua;
	sol::protected_function seqFunction;
	bool hasFunction = false;

	Point2d<FixedPoint> root; //Character (x,y) position in game. Every box position is relative to this.
	Point2d<FixedPoint> vel;
	Point2d<FixedPoint> accel;

	Frame *framePointer;
	int currSeq = 0; //The active sequence.
	int currFrame = 0;
	int frameDuration; //counter for changing frames
	int loopCounter = 0;
	int hitstop = 0; //hitstop counter
};

#endif /* ACTOR_H_GUARD */
