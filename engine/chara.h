#ifndef CHARACTER_H_INCLUDED
#define CHARACTER_H_INCLUDED

#include "camera.h"
#include "command_inputs.h"
#include "fixed_point.h"
#include <geometry.h>

#include <deque>
#include <string>
#include <vector>
#include <utility>

#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>

#include <sol/sol.hpp>

namespace flag
{
	enum //frame-dependent bit-mask flags
	{
		canMove = 0x1,
		dontWalk = 0x2,
		
		startHit = 0x8000'0000
	};
}

namespace jump
{
	enum
	{
		none = 0,
		frame,
		loop,
		seq
	};
}

struct CharFileHeader //header for .char files.
{
	char signature[32];
	uint32_t version;
	uint16_t sequences_n;
};

struct Frame_property
{
	int32_t spriteIndex = 0;
	int32_t duration = 0;
	int32_t jumpTo = 0;
	int32_t jumpType = 0;
	int32_t relativeJump = false;

	uint32_t flags = 0;
	int32_t vel[2] = {0}; // x,y
	int32_t accel[2] = {0};
	int32_t movementType[2] = {0}; //0 None. 1 Set-set. 2 Add-set. 3 Add-add

	int16_t cancelType[2] = {};
	int32_t state = 0;
	
	float spriteOffset[2]; //x,y
	int16_t loopN;
	int16_t chType;
	float scale[2];
	float color[4];
	int32_t blendType = 0;
	float rotation[3]; //XYZ
};

struct Frame
{
	
	Frame_property frameProp;
	//Attack_property attackProp;
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
	std::string function;
};

class Character
{
public: //access only, do not change outside the class.
	std::vector<Sequence> sequences;

private:
	sol::state lua;
	sol::function seqFunction;
	sol::function updateFunction;
	bool hasFunction = false, hasUpdate = false;

	Camera *currView;

	int health = 10000;
	//int hitsTaken;
	//inr damageTaken;

	Point2d<FixedPoint> root; //Character (x,y) position in game. Every box position is relative to this.
	
	Frame *framePointer;
	Frame *hitTargetFrame;

	CommandInputs cmd;

	int currSeq = 0; //The active sequence.
	int currFrame = 0;
	int frameDuration; //counter for changing frames
	int loopCounter = 0;
	int hitstop = 0; //hitstop counter
	unsigned int lastKey = 0;

	int totalSubframeCount = 0;
	int	subframeCount = 0;

	int side; //used to invert the x of all sort of things

	Point2d<FixedPoint> vel;
	Point2d<FixedPoint> accel;
	FixedPoint impulses[2];//X speed set by outside forces.
	
	Character* target;

	//These are set by/on the target.
	bool alreadyHit = false; //Check for avoiding the same c-frame hitting more than once. Resets when c-frame advances.
	bool comboFlag = false; //Sets when hit is sucessful. Resets when sequence changes. Used for cancelling purposes.
	//This is set only by self.
	bool gotHit = false; //Sets when getting hit (doesn't matter if you block). Resets when the hit is processed in Update().
	bool interrumpible = false;
	bool mustTurnAround = false;

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

	void Input(input_deque &keyPresses);

	void Update();

private:
	void ScriptSetup();
	void Translate(Point2d<FixedPoint> amount);
	void Translate(FixedPoint x, FixedPoint y);

	bool SuggestSequence(int seq); //returns true on success

	void GotoSequence(int seq);
	void GotoFrame(int frame);

	void ResolveHit(int keypress);
	bool TurnAround(int sequence = -1);

	enum state
	{
		stand,
		crouch,
		air,
		otg,
	};
};

#endif // CHARACTER_H_INCLUDED
