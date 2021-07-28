#ifndef ACTOR_H_GUARD
#define ACTOR_H_GUARD

#include "framedata.h"
#include <geometry.h>
#include <fixed_point.h>
#include <unordered_map>
#include <sol/sol.hpp>
#include <glm/mat4x4.hpp>

const FixedPoint floorPos(32);

struct HitDef
{
	struct VectorTable
	{
		int maxPushBackTime;
		int xSpeed, ySpeed;
		int xAccel, yAccel;
		std::string sequenceName;
	};
	//key is type (air, cro, sta), array value is vector subtable (hit and block)
	std::unordered_map<int, std::array<VectorTable, 2>> vectorTables;
	int attackFlags = 0;
	int damage = 0;
	int guardDamage = 0;
	int correction = 0;
	int correctionType = 0;
	int meterGain = 0;
	int hitStop = 0;
	int blockStop = -1;
	int untech = 0;
	int blockstun = 0; //Untech and block
	int priority = 0;
	int soundFx = 0;
	int hitFx = 0;

	void Clear();
	void SetVectors(int state, sol::table onHit, sol::table onBlock);
};

class Actor{
	friend class Character;
	sol::state &lua;
	std::vector<Sequence> &sequences;

protected:
	std::list<Actor> children;
	std::list<Actor>::iterator myPos;
	Actor* parent = nullptr;
	HitDef attack;
	//sol::state &lua;

	Point2d<FixedPoint> root; //Character (x,y) position in game. Every box position is relative to this.
	Point2d<FixedPoint> vel;
	Point2d<FixedPoint> accel;
	Sequence *seqPointer;
	Frame *framePointer;
	
	int side = 1; //used to invert the x of all sort of things

	int currSeq = 0; //The active sequence.
	int currFrame = 0;
	int frameDuration; //counter for changing frames
	int loopCounter = 0;
	int totalSubframeCount = 0;
	int	subframeCount = 0;
	int hitstop = 0; //hitstop counter

	int hitCount = 0;
	bool hittable = false;
	enum hitType {
		none,
		hurt,
		blocked
	};
	//comboType is set to unresolved if it connects, and then to hurt/blocked by the target. Resets when sequence changes. Used for cancelling purposes.
	int comboType = none; 
	//Set when getting hit (doesn't matter if you block). Resets when the hit is resolved.
	bool gotHit = false; 
	sol::table userData;

public:
	Actor(std::vector<Sequence> &sequences, sol::state &lua);

	virtual void Update();
	void GotoSequence(int seq);
	bool GotoFrame(int frame);
	void Translate(Point2d<FixedPoint> amount);
	void Translate(FixedPoint x, FixedPoint y);
	void SetSide(int side);
	int GetSide();

	Actor& SpawnChild(int sequence = 0);
	void KillSelf();

	Frame *GetCurrentFrame();
	int GetSpriteIndex();
	glm::mat4 GetSpriteTransform();

	static bool HitCollision(const Actor& hurt, const Actor& hit);
	static void DeclareActorLua(sol::state &lua);

	void GetAllChildren(std::list<Actor*> &list, bool includeSelf = true);

protected:
	void SeqFun();
	void SetHitDef(sol::table onHit, sol::table onBlock);
	virtual int ResolveHit(int keypress, Actor *hitter);
};

#endif /* ACTOR_H_GUARD */