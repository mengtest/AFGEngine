#ifndef CHARACTER_H_INCLUDED
#define CHARACTER_H_INCLUDED

#include "battle_interface.h"
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
	friend class Player;
	Character *target = nullptr;
	//sol::state lua;

	int health = 10000;
	//int hitsTaken;
	//inr damageTaken;
	int hurtSeq = -1;
	bool gotHit = false;

	bool blockFlag = false;
	int hitFlags = 0; //When getting hit

	HitDef::Vector bounceVector;
	int blockTime = 0;
	int pushTimer = 0; //Counts down the pushback time.

	std::reference_wrapper<BattleInterface> scene;
	
	bool interrumpible = false;
	bool mustTurnAround = false;

	//FixedPoint getAway; //Amount to move after collision
	FixedPoint touchedWall; //left wall: -1, right wall = 1, no wall = 0;
	MotionData lastCommand;


public:
	Character(FixedPoint posX, int side, BattleInterface& scene, sol::state &lua, std::vector<Sequence> &sequences, std::vector<Actor> &actorList);
	bool Update();
	
	void BoundaryCollision(); //Collision against stage
	void Input(input_deque &keyPresses, CommandInputs &cmd);

private:
	
	void Translate(Point2d<FixedPoint> amount);
	void Translate(FixedPoint x, FixedPoint y);
	void GotoSequence(int seq);
	void GotoSequenceMayTurn(int seq);
	int ResolveHit(int keypress, Actor *hitter);
	bool TurnAround(int sequence = -1);
};


struct PlayerStateCopy
{
	sol::global_table lua_state;
	std::unique_ptr<Character> charObj;
	std::vector<Actor> children;
	Character* target;
	input_deque keyBufOrig;
	input_deque keyBufDelayed;
	unsigned int lastKey[2]{};
};

class Player
{
private:
	sol::state lua;
	sol::protected_function updateFunction;
	bool hasUpdateFunction = false;

	std::vector<Sequence> sequences;
	BattleInterface& scene;
	Character* charObj = nullptr;
	Character* target;

	int delay = 0;
	input_deque keyBufOrig;
	input_deque keyBufDelayed;
	unsigned int lastKey[2]{};
	CommandInputs cmd;

	bool ScriptSetup();
	
public:
	std::vector<Actor*> updateList;
	std::vector<Actor> children;
	Player(BattleInterface& scene);
	~Player();
	Player(int side, std::string charFile, BattleInterface& scene);
	void Load(int side, std::string charFile);
	void SetState(PlayerStateCopy &state);
	PlayerStateCopy GetStateCopy();

	void SetTarget(Player &target);
	void Update(HitboxRenderer &hr);
	void ProcessInput();
	Point2d<FixedPoint> GetXYCoords();
	float GetHealthRatio();
	void SetDelay(int delay);
	void SendInput(int key);

	static void HitCollision(Player &blue, Player &red); //Checks hit/hurt box collision and sets flags accordingly.
	static void Collision(Player &blue, Player &red); //Detects and resolves collision between characters and/or the camera.
};



#endif // CHARACTER_H_INCLUDED
