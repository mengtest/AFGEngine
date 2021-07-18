#include <cmath>
#include <deque>
#include <fstream>
#include <string>
#include <iostream>

#include <glad/glad.h>	//These shouldn't be here but w/e for now

#include "chara.h"
#include "chara_input.h"
#include "raw_input.h" //Used only by Character::ResolveHit

#include <sol/sol.hpp>

bool Character::isColliding;
const FixedPoint floorPos(32);

#define rv(X) ((char*)&X)
#define rptr(X) ((char*)X)

struct BoxSizes
{
	int8_t greens;
	int8_t reds;
	int8_t collision;
};

Character::Character(FixedPoint xPos, float _side, std::string charFile) :
	health(10000),
	actTableG{0},
	actTableA{0},
	currSeq(0),
	currFrame(0),
	hitstop(0),
	side(_side),
	painType(0),
	alreadyHit(false),
	isKickingAss(false),
	gotHit(0),
	touchedWall(0)
{
	constexpr const char *charSignature = "AFGECharacterFile";
	constexpr uint32_t currentVersion = 99'2;

	root.x = xPos;
	root.y = floorPos;

	//loads character from a file and fills sequences/frames and all that yadda.
	std::ifstream file(charFile, std::ios_base::in | std::ios_base::binary);
	if (!file.is_open())
	{
		std::cerr << "Couldn't open character file.\n";
	}

	CharFileHeader header;
	file.read(rv(header), sizeof(CharFileHeader));
	if(strcmp(charSignature, header.signature))
	{
		std::cerr << "Signature mismatch.\n";
	}
	if(header.version != currentVersion)
	{
		std::cerr << "Format version mismatch.\n";
	}

	sequences.resize(header.sequences_n);
	for (uint16_t i = 0; i < header.sequences_n; ++i)
	{
		auto &currSeq = sequences[i];
		uint8_t namelength;
		file.read(rv(namelength), sizeof(namelength));
		file.ignore(namelength);

		file.read(rv(currSeq.props), sizeof(seqProp));

		uint8_t seqlength;
		file.read(rv(seqlength), sizeof(seqlength));
		currSeq.frames.resize(seqlength);
		for (uint8_t i2 = 0; i2 < seqlength; ++i2)
		{
			auto &currFrame = currSeq.frames[i2];

			BoxSizes bs;
			file.read(rv(bs), sizeof(BoxSizes));
			std::vector<int> greens(bs.greens);
			std::vector<int> reds(bs.reds);
			std::vector<int> collision(bs.collision);

			file.read(rv(currFrame.frameProp), sizeof(Frame_property));
			file.read(rv(currFrame.attackProp), sizeof(Attack_property));

			file.read(rptr(greens.data()), sizeof(int) * bs.greens);
			file.read(rptr(reds.data()), sizeof(int) * bs.reds);
			file.read(rptr(collision.data()), sizeof(int) * bs.collision);

			for(int bi = 0; bi < bs.greens; bi+=4)
			{
				currFrame.greenboxes.push_back(
					Rect2d<FixedPoint>(
						Point2d<FixedPoint>(greens[bi+0],greens[bi+1]),
						Point2d<FixedPoint>(greens[bi+2],greens[bi+3]))
				);
			}
			for(int bi = 0; bi < bs.reds; bi+=4)
			{
				currFrame.redboxes.push_back(
					Rect2d<FixedPoint>(
						Point2d<FixedPoint>(reds[bi+0],reds[bi+1]),
						Point2d<FixedPoint>(reds[bi+2],reds[bi+3]))
				);
			}

			if(bs.collision>0)
			{
				sequences[i].frames[i2].colbox.bottomLeft.x = collision[0];
				sequences[i].frames[i2].colbox.bottomLeft.y = collision[1];
				sequences[i].frames[i2].colbox.topRight.x = collision[2];
				sequences[i].frames[i2].colbox.topRight.y = collision[3];
			}
		}
	}

	file.close();

	sol::state lua;
	auto result = lua.script_file("data/char/vaki/moves.lua");
	if(!result.valid()){
		sol::error err = result;
		std::cerr << "The code has failed to run!\n"
		          << err.what() << "\nPanicking and exiting..."
		          << std::endl;
	}
	sol::table actTable = lua["actTable"];
	for(const auto &val : actTable)
	{
		int index = val.first.as<int>();
		sol::table arr = val.second;
		actTableG[index] = arr[2];
		actTableA[index] = arr[3];
	}

	selectedMotionList = motionListDataG;
	selectedTable = actTableG;
	GotoSequence(actTableG[0]);
	GotoFrame(0);
	//TransitionInto(state::GROUNDED);
	return;
}

void Character::Print()
{
	std::cout << "side: " << side << "  X: " << (float)root.x << "\n";
}

Point2d<FixedPoint> Character::getXYCoords()
{
	return Point2d<FixedPoint>(root.x, root.y);
}

void Character::SetCameraRef(Camera *ref)
{
	currView = ref;
}

void Character::setTarget(Character *t)
{
	target = t;
}

int Character::GetSpriteIndex()
{
	return framePointer->frameProp.spriteIndex;
}

glm::mat4 Character::GetSpriteTransform()
{
	glm::mat4 tranform(1);
	
	tranform = glm::scale(tranform, glm::vec3(side, 1.f, 1.f));
	tranform = glm::translate(tranform, glm::vec3((root.x)*side-128.f, root.y-40.f, 0));
	
	
	return tranform;
}

void Character::Collision(Character *blue, Character *red)
{
	isColliding = false;

	Rect2d<FixedPoint> colBlue = blue->framePointer->colbox;
	Rect2d<FixedPoint> colRed = red->framePointer->colbox;

	if (blue->side == -1)
		colBlue = colBlue.FlipHorizontal();
	if (red->side == -1)
		colRed = colRed.FlipHorizontal();

	colBlue = colBlue.Translate(blue->root);
	colRed = colRed.Translate(red->root);

	isColliding = colBlue.Intersects(colRed);

	if (isColliding)
	{
		const FixedPoint magic(480);

		FixedPoint getAway(0,5);

		if (blue->touchedWall != 0 || red->touchedWall != 0)
			getAway = 1;

		if (blue->root.x + blue->touchedWall * magic < red->root.x + red->touchedWall * magic)
		{
			getAway = getAway * (colBlue.topRight.x - colRed.bottomLeft.x);

			blue->getAway = -getAway;
			red->getAway = getAway;

		}
		else
		{
			getAway = getAway * (colRed.topRight.x - colBlue.bottomLeft.x);

			blue->getAway = getAway;
			red->getAway = -getAway;
		}

		blue->Translate(blue->getAway, 0);
		red->Translate(red->getAway, 0);
	}

	return;
}

void Character::HitCollision()
{
	bool isHit = false;
	for(auto hurtbox : framePointer->greenboxes)
	{
		if(side == -1)
			hurtbox = hurtbox.FlipHorizontal();
		hurtbox = hurtbox.Translate(root);
		for(auto hitbox : target->framePointer->redboxes)
		{
			if(target->side == -1)
				hitbox = hitbox.FlipHorizontal();
			hitbox = hitbox.Translate(target->root);
			if(hitbox.Intersects(hurtbox))
			{
				isHit = true;
				goto break_from_all;
			}
		}
	}

break_from_all:
	if (isHit)
	{
		if (target->alreadyHit) //Avoids the same frame hitting multiple times.
			return;

		target->isKickingAss = true;
		target->alreadyHit = true;
		gotHit = true;
		hitTargetFrame = target->framePointer;
	}
	return;
}

void Character::BoundaryCollision()
{
	const FixedPoint wallOffset(10);
	touchedWall = 0;

	if (root.x <= currView->GetWallPos(camera::leftWall) + wallOffset)
	{
		touchedWall = -1;
		root.x = currView->GetWallPos(camera::leftWall) + wallOffset;
	}

	else if (root.x >= currView->GetWallPos(camera::rightWall) - wallOffset)
	{
		touchedWall = 1;
		root.x = currView->GetWallPos(camera::rightWall) - wallOffset;
	}

	if (touchedWall == target->touchedWall) //Someone already has the wall.
		touchedWall = 0;
}

float Character::getHealthRatio()
{
	if (health < 0)
		health = 10000;
	return health * (1.f / 10000.f);
}

bool Character::SuggestSequence(int seq)
{
	if(seq == -1)
		return false;

	//Checks if it should ignore the next command
	//if not cancellable. Do nothing
	//if next seq is not attack do nothing <- Define in movelist
	
	if (currSeq == seq) //The sequence can't go to itself (unless the sprite facing side hasn't been updated???)
	{
		return false;
	}
	//If you're in pain. Do nothing?

	//Attack cancel rules and all that stuff go here.
	GotoSequence(seq);
	return true;
}

void Character::GotoSequence(int seq)
{
	if (seq < 0)
		return;

	isKickingAss = false;
	currSeq = seq;
	currFrame = 0;
	GotoFrame(0);
}

void Character::GotoFrame(int frame)
{

	alreadyHit = false;
	currFrame = frame;
	framePointer = &sequences[currSeq].frames[currFrame];

	if (framePointer->frameProp.flags & flag::RESET_INFLICTED_VEL)
	{
		for (int i = 0; i <= 1; ++i)
		{
			impulses[i] = 0;
		}
	}

	if (!(framePointer->frameProp.flags & flag::KEEP_VEL))
	{
		vel.x.value = (framePointer->frameProp.vel[0] * side)*100;
		vel.y.value = (framePointer->frameProp.vel[1])*100;
	}
	if (!(framePointer->frameProp.flags & flag::KEEP_ACC))
	{
		accel.x.value = (framePointer->frameProp.accel[0] * side)*100;
		accel.y.value = framePointer->frameProp.accel[1]*100;
	}
	//std::cout << sequences[currSeq].trigger << ": " << framePointer->frameProp.vel.x << "\t" << frame  << "\n";

	frameDuration = sequences[currSeq].frames[currFrame].frameProp.duration;
}


void Character::ResolveHit(int keypress) //key processing really shouldn't be here.
{
	if (gotHit)
	{
		target->hitstop = hitstop = hitTargetFrame->attackProp.stop[0];

		int left;
		int right;
		if (side == -1) //Inverts absolute input depending on side. Apparent input is corrected.
		{
			left = key::buf::RIGHT;
			right = key::buf::LEFT;
		}
		else
		{
			left = key::buf::LEFT;
			right = key::buf::RIGHT;
		}

		bool blocked = false;
		/* if (currentState == state::GROUNDED && (keypress & left) && !(keypress & right))
		{
			if ((keypress & key::buf::DOWN) && (hitTargetFrame->frameProp.flags & flag::CROUCH_BLOCK))
			{
				GotoSequence(actTableG[act::GUARD1]);
				frameDuration = hitTargetFrame->frameProp.blockstun;
				blocked = true;
			}
			else if (!(keypress & key::buf::DOWN) && (hitTargetFrame->frameProp.flags & flag::STAND_BLOCK))
			{
				if (framePointer->frameProp.state == state::fr::CROUCHED)
					GotoSequence(selectedTable[act::GUARD1]);
				else
					GotoSequence(selectedTable[act::GUARD4]);
				frameDuration = hitTargetFrame->frameProp.blockstun;
				blocked = true;
			}
		}

		if (!blocked)
		{
			if (framePointer->frameProp.state == state::fr::AIRBORNE)
			{
				if (hitTargetFrame->frameProp.painType == pain::HIGH)
					GotoSequence(actTableA[act::HIGH_PAIN]);
				else
					GotoSequence(actTableA[act::MID_PAIN]);
			}
			else if (framePointer->frameProp.state != state::fr::OTG)
			{
				if (framePointer->frameProp.state == state::fr::CROUCHED)
					GotoSequence(actTableG[act::LOW_PAIN]);
				else if (hitTargetFrame->frameProp.painType == pain::HIGH)
					GotoSequence(actTableG[act::HIGH_PAIN]);
				else
					GotoSequence(actTableG[act::MID_PAIN]);
			}
		}

		if (blocked)
		{
			health -= hitTargetFrame->frameProp.damage[3];
			target->impulses[PUSHBACKx] = hitTargetFrame->frameProp.pushback[1] * -target->side*0.5;
		}
		else
		{
			frameDuration = hitTargetFrame->frameProp.hitstun;
			health -= hitTargetFrame->frameProp.damage[2];
			target->impulses[PUSHBACKx] = hitTargetFrame->frameProp.pushback[0] * -target->side*0.5;
		}

		if (touchedWall != 0)
		{
			impulses[HITPUSHx] = hitTargetFrame->frameProp.push[0] * -side;
			if (impulses[HITPUSHx] * -side <= FixedPoint(0)) //Doesn't get pushed when touching wall, only pulled.
				impulses[HITPUSHx] = 0;
		}
		else
		{
			impulses[HITPUSHx] = hitTargetFrame->frameProp.push[0] * target->side;
		} */

		//TODO: Remove this nasty hack
		//vel.y = hitTargetFrame->frameProp.push[1];

		gotHit = false;
	}
}

void Character::Translate(Point2d<FixedPoint> amount)
{
	root += amount;
	BoundaryCollision();
}

void Character::Translate(FixedPoint x, FixedPoint y)
{
	root.x += x;
	root.y += y;
	BoundaryCollision();
}

void Character::Update()
{
	if (hitstop > 0)
	{
		--hitstop;
		return;
	}

	//Only if frame is standing, walking or crouching
	if (root.x < target->root.x) //Side switching.
	{
		if (side == -1)
		{
			if (framePointer->frameProp.state == state::fr::CROUCHED)
				SuggestSequence(actTableG[act::CROUCH_180]);
			else if (actTableG[act::NEUTRAL] == currSeq)
				SuggestSequence(actTableG[act::STAND_180]);
		}
		side = 1;
	}
	else if (root.x > target->root.x)
	{
		if (side == 1)
		{
			if (framePointer->frameProp.state == state::fr::CROUCHED)
				SuggestSequence(actTableG[act::CROUCH_180]);
			else if (actTableG[act::NEUTRAL] == currSeq)
				SuggestSequence(actTableG[act::STAND_180]);
		}
		side = -1;
	}

	if (touchedWall != 0)
	{
		target->root.x += -impulses[0];
	}

	vel += accel;
	Translate(vel);

	if (root.y < floorPos) //Check collision with floor
	{
		vel.y = 0;
		root.y = floorPos;
		//Jump to landing frame.
	}

	--frameDuration;
	if (frameDuration == 0)
	{
		if (currFrame < sequences[currSeq].frames.size() - 1)
		{
			currFrame += 1;
			GotoFrame(currFrame);
		}
		else //Go to default sequence if nothing happens.
			GotoSequence(0);
	}
}

void Character::Input(input_deque *keyPresses)
{
	ResolveHit((*keyPresses)[0]);

	if (framePointer->frameProp.flags & flag::IGNORE_INPUT)
		return;

	int command = InstantInput(keyPresses, side, selectedMotionList);
	if (command > 0x1000)
	{
		SuggestSequence(command - 0x1000);
		return;
	}

	if (command == act::NOTHING)
		return;

	int nextSeq = selectedTable[command];

	if (framePointer->frameProp.state == state::fr::CROUCHED)
	{
		if (nextSeq == actTableG[act::DOWN])
			return;
		else if (nextSeq == actTableG[act::NEUTRAL])
			nextSeq = actTableG[act::STAND_UP];
	}

	SuggestSequence(nextSeq);
}
