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
	//These are set by/on the target.
	bool alreadyHit = false; //Check for avoiding the same c-frame hitting more than once. Resets when c-frame advances.
	bool comboFlag = false; //Sets when hit is sucessful. Resets when sequence changes. Used for cancelling purposes.
	//This is set only by self.
	bool gotHit = false; //Sets when getting hit (doesn't matter if you block). Resets when the hit is processed in Update().
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

	static void Collision(Character* blue, Character* red); //Detects and resolves collision between characters and/or the camera.
	void BoundaryCollision();
	void HitCollision(); //Checks collision between own green boxes and target's red boxes.

	void Input(input_deque &keyPresses);

private:
	void ScriptSetup();
	void Translate(Point2d<FixedPoint> amount);
	void Translate(FixedPoint x, FixedPoint y);

	bool SuggestSequence(int seq); //returns true on success

	void GotoSequence(int seq);
	void GotoFrame(int frame);

	void ResolveHit(int keypress);
	bool TurnAround(int sequence = -1);
};

#endif // CHARACTER_H_INCLUDED
