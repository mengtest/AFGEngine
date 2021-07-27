#ifndef CHARACTER_H_INCLUDED
#define CHARACTER_H_INCLUDED

#include "framedata.h"
#include "camera.h"
#include "command_inputs.h"
#include "fixed_point.h"
#include "actor.h"
#include <geometry.h>

#include <deque>
#include <string>
#include <vector>

#include <sol/sol.hpp>

class Character : public Actor
{
private:
	sol::state lua;
	sol::protected_function updateFunction;
	bool hasUpdateFunction = false;

	std::vector<Sequence> sequences;

	int health = 10000;
	//int hitsTaken;
	//inr damageTaken;

	FixedPoint impulses[2];//X speed set by outside forces. Pushback?

	CommandInputs cmd;
	unsigned int lastKey = 0;

	Camera *currView;
	Character* target;

	bool interrumpible = false;
	bool mustTurnAround = false;

	static bool isColliding;
	//FixedPoint getAway; //Amount to move after collision
	FixedPoint touchedWall; //left wall: -1, right wall = 1, no wall = 0;

public:
	Character(FixedPoint posX, float side, std::string charFile);

	void Update();

	float getHealthRatio();
	Point2d<FixedPoint> getXYCoords();

	void SetCameraRef(Camera *ref);
	void setTarget(Character* target);

	void BoundaryCollision(); //Collision against stage
	static void Collision(Character* blue, Character* red); //Detects and resolves collision between characters and/or the camera.
	static void HitCollision(Character &blue, Character &red, int blueKey, int redKey); //Checks hit/hurt box collision and sets flags accordingly.

	void Input(input_deque &keyPresses);

private:
	void ScriptSetup();
	void Translate(Point2d<FixedPoint> amount);
	void Translate(FixedPoint x, FixedPoint y);

	bool SuggestSequence(int seq); //returns true on success

	void GotoSequence(int seq);
	int ResolveHit(int keypress, Actor *hitter);
	bool TurnAround(int sequence = -1);
};

#endif // CHARACTER_H_INCLUDED
