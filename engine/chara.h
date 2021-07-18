#ifndef CHARACTER_H_INCLUDED
#define CHARACTER_H_INCLUDED
#include <geometry.h>

#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>

#include <deque>
#include <string>
#include <vector>
#include <utility>
#include <unordered_map>

typedef std::unordered_map<std::string, uint16_t> NameMap;

#include "camera.h"
#include "chara_input.h"
#include "fixed_point.h"

namespace flag
{
	enum //frame-dependent bit-mask flags
	{
		FRICTION = 0x1,
		GRAVITY = 0x2,
		KEEP_VEL = 0x4,
		KEEP_ACC = 0x8,
		CROUCH_BLOCK = 0x10,
		STAND_BLOCK = 0x20,
		AIR_BLOCK = 0x40,
		SINGLE_HIT = 0x80,
		CANCELLABLE = 0x100,
		CANCEL_WHIFF = 0x200,
		JUMP_C_HIT = 0x400,
		JUMP_C_BLOCK = 0x800,
		_UNUSED13 = 0x1000,
		_UNUSED14 = 0x2000,
		_UNUSED15 = 0x4000,
		IGNORE_INPUT = 0x8000,
		RESET_INFLICTED_VEL = 0x10000,
		_UNUSED18 = 0x20000, //No sprite mirroring (used only by frame 0)
	};
}

namespace pain
{
	enum
	{
		HIGH,
		LOW,
	};
}

namespace state
{
	enum //Seq-wise. Probably getting scrapped.
	{
		GROUNDED,
		AIRBORNE,
		BUSY_GRND,
		BUSY_AIR,
		PAIN_GRND,
		PAIN_AIR,
	};

	namespace fr //frame
	{
		enum
		{
			STANDING,
			CROUCHED,
			AIRBORNE,
			OTG,
		};
	}
}

struct CharFileHeader //header for .char files.
{
	char signature[32];
	uint32_t version;
	uint16_t sequences_n;
};

struct Motion_Data
{
	int bufLen = 0;
	int seqRef = 0;
	std::string motionStr; //Without the button press.
	char button = 0;
};

struct Attack_property
{
	uint32_t attackFlags = 0;
	int damage[3] = {0}; // Perma damage, Red damage, Guard damage
	int correction = 0;
	int correctionType = 0;
	int meterGain = 0;
	int stopType = 0;
	int stop[2] = {0}; //Hit block
	int stun[2] = {0}; //Untech and block
	int vectorId[6]; //SCA hit block
	int priority = 0;
	int soundFx = 0;
	int hitFx = 0;
};

struct Frame_property
{
	int spriteIndex = 0;
	int duration = 0;
	int jumpTo = 0;
	int jumpType = 0;
	bool relativeJump = false;

	uint32_t flags = 0;
	int vel[2] = {0}; // x,y
	int accel[2] = {0};
	int movementType[2] = {0}; //Add or set X,Y

	int cancelType = 0;
	int state = 0;
	
	float spriteOffset[2]; //x,y
	float rotation;
	float scale[2];
	float color[4];
	int blendType = 0;
};

struct Frame
{
	
	Frame_property frameProp;
	Attack_property attackProp;
	//Boxes are defined by BL, BR, TR, TL points, in that order.
	typedef std::vector<Rect2d<FixedPoint>> boxes_t;
	boxes_t greenboxes;
	boxes_t redboxes;
	Rect2d<FixedPoint> colbox;
};

struct seqProp
{
	int level = 0;
	int landFrame = 0;
	int zOrder = 0;
};

struct Sequence
{
	seqProp props;
	std::vector<Frame> frames;
	std::string name;
};

class Character
{
public: //access only, do not change outside the class.
	std::vector<Sequence> sequences;

private:
	Camera *currView;

	int health;
	//int hitsTaken;
	//inr damageTaken;

	Point2d<FixedPoint> root; //Character (x,y) position in game. Every box position is relative to this.
	
	Frame *framePointer;
	Frame *hitTargetFrame;

	int actTableG[64]; //Array to translate act:: constants to their assigned sequence.
	int actTableA[64]; //Aerial counterpart
	Motion_Data motionListDataG[32]; //List of motion inputs. Not only for special attack usage.
	Motion_Data motionListDataA[32];

	int *selectedTable;
	Motion_Data *selectedMotionList;

	int currSeq; //The active sequence.
	int currFrame;
	int frameDuration; //counter for changing frames

	int hitstop; //hitstop counter

	int side; //used to invert the x of all sort of things

	Point2d<FixedPoint> vel;
	Point2d<FixedPoint> accel;
	FixedPoint impulses[2];//X speed set by outside forces.
	
	Character* target;
	int painType;
	//These are set by/on the target.
	bool alreadyHit; //Check for avoiding the same c-frame hitting more than once. Resets when c-frame advances.
	bool isKickingAss; //Sets when hit is sucessful. Resets when sequence changes. Used for cancelling purposes.
	//This is set only by self.
	bool gotHit; //Sets when getting hit (doesn't matter if you block). Resets when the hit is processed in Update().

	static bool isColliding;
	FixedPoint getAway; //Amount to move after collision
	FixedPoint touchedWall; //left wall: -1, right wall = 1, no wall = 0;

public:
	Character(FixedPoint posX, float side, std::string charFile);

	void Print();

	float getHealthRatio();

	static void Collision(Character* blue, Character* red); //Detects and resolves collision between characters and/or the camera.
	void BoundaryCollision();
	void HitCollision(); //Checks collision between own green boxes and target's red boxes.

	Point2d<FixedPoint> getXYCoords();

	void SetCameraRef(Camera *ref);
	void setTarget(Character* target);

	int GetSpriteIndex();
	glm::mat4 GetSpriteTransform();

	void Input(input_deque *keyPresses);

	void Update();

private:
	void Translate(Point2d<FixedPoint> amount);
	void Translate(FixedPoint x, FixedPoint y);

	bool SuggestSequence(int seq); //returns true on success

	void GotoSequence(int seq);
	void GotoFrame(int frame);

	void ResolveHit(int keypress);
};

#endif // CHARACTER_H_INCLUDED
