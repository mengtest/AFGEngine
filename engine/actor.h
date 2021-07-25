#ifndef ACTOR_H_GUARD
#define ACTOR_H_GUARD

#include "framedata.h"
#include <geometry.h>
#include <fixed_point.h>
#include <sol/sol.hpp>

#include <glm/mat4x4.hpp>

const FixedPoint floorPos(32);

class Actor{
	friend class Character;
private:
	std::vector<Sequence> &sequences;
	std::list<Actor> children;
	std::list<Actor>::iterator myPos;
	Actor* parent = nullptr;
	//sol::state &lua;

	Point2d<FixedPoint> root; //Character (x,y) position in game. Every box position is relative to this.
	Point2d<FixedPoint> vel;
	Point2d<FixedPoint> accel;
	Sequence *seqPointer;
	Frame *framePointer;
	
	int side; //used to invert the x of all sort of things

	int currSeq = 0; //The active sequence.
	int currFrame = 0;
	int frameDuration; //counter for changing frames
	int loopCounter = 0;
	int totalSubframeCount = 0;
	int	subframeCount = 0;
	int hitstop = 0; //hitstop counter

public:
	Actor(std::vector<Sequence> &sequences);

	void Update();
	void GotoSequence(int seq);
	bool GotoFrame(int frame);
	void Translate(Point2d<FixedPoint> amount);
	void Translate(FixedPoint x, FixedPoint y);
	void SetSide(int side);
	int GetSide();

	Actor& SpawnChild();
	void KillSelf();

	Frame *GetCurrentFrame();
	int GetSpriteIndex();
	glm::mat4 GetSpriteTransform();

	static bool HitCollision(const Actor& hurt, const Actor& hit);
	static void DeclareActorLua(sol::state &lua);

private:
	void SeqFun();
};

#endif /* ACTOR_H_GUARD */
