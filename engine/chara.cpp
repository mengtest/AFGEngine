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
	spriteSide(_side),
	side(_side),
	painType(0),
	alreadyHit(false),
	isKickingAss(false),
	gotHit(0),
	gravity(0.5),
	friction(0.05),
	touchedWall(0)
{
	constexpr const char *charSignature = "AFGECharacterFile";
	constexpr uint32_t currentVersion = 99'1;

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

	{//Remove
		int motionLenG;
		int motionLenA;

		file.read((char *)&motionLenG, sizeof(int));
		file.read((char *)&motionLenA, sizeof(int));

		for (int i = 0; i < motionLenG; ++i)
		{
			file.read((char *)&motionListDataG[i].bufLen, sizeof(int));
			file.read((char *)&motionListDataG[i].seqRef, sizeof(int));

			int strSize;
			file.read((char *)&strSize, sizeof(int));

			std::string fullMotionStr(strSize, '\0');

			file.read((char *)fullMotionStr.data(), strSize);
			file.read(&motionListDataG[i].button, sizeof(char));

			motionListDataG[i].motionStr = fullMotionStr;
		}

		for (int i = 0; i < motionLenA; ++i)
		{
			file.read((char *)&motionListDataA[i].bufLen, sizeof(int));
			file.read((char *)&motionListDataA[i].seqRef, sizeof(int));

			int strSize;
			file.read((char *)&strSize, sizeof(int));

			std::string fullMotionStr(strSize, '\0');

			file.read((char *)fullMotionStr.data(), strSize);
			file.read(&motionListDataA[i].button, sizeof(char));

			motionListDataA[i].motionStr = fullMotionStr;
		}
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

			file.read(rv(currFrame.spriteIndex), sizeof(int));
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
	return framePointer->spriteIndex;
}

glm::mat4 Character::GetSpriteTransform()
{
	glm::mat4 tranform(1);
	
	tranform = glm::scale(tranform, glm::vec3(spriteSide, 1.f, 1.f));
	tranform = glm::translate(tranform, glm::vec3((root.x)*spriteSide-128.f, root.y-40.f, 0));
	
	
	return tranform;
}

void Character::Collision(Character *blue, Character *red)
{
	isColliding = false;

	Rect2d<FixedPoint> colBlue = blue->framePointer->colbox;
	Rect2d<FixedPoint> colRed = red->framePointer->colbox;

	if (blue->spriteSide == -1)
		colBlue = colBlue.FlipHorizontal();
	if (red->spriteSide == -1)
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
		if(spriteSide == -1)
			hurtbox = hurtbox.FlipHorizontal();
		hurtbox = hurtbox.Translate(root);
		for(auto hitbox : target->framePointer->redboxes)
		{
			if(target->spriteSide == -1)
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

	if (sequences[currSeq].props.machineState == state::BUSY_GRND) //Checks if it should ignore the next command
	{
		if (!isKickingAss && !(framePointer->frameProp.flags & flag::CANCEL_WHIFF))
			return false;
		if (!(framePointer->frameProp.flags & flag::CANCELLABLE)) //if not cancellable
			return false;
		if (!(sequences[seq].props.machineState == state::BUSY_GRND || sequences[seq].props.machineState == state::BUSY_AIR)) //if next is not attack
			return false;
		GotoSequence(seq);
		return true;
	}
	else if (sequences[currSeq].props.machineState == state::BUSY_AIR)
	{
		if (!isKickingAss && !(framePointer->frameProp.flags & flag::CANCEL_WHIFF))
			return false;
		if (!(framePointer->frameProp.flags & flag::CANCELLABLE)) //if not cancellable
			return false;
		if (sequences[seq].props.machineState != state::BUSY_AIR) //if next is not attack
			return false;
		GotoSequence(seq);
		return true;
	}

	if (currSeq == seq && spriteSide == side) //The sequence can't go to itself unless the sprite facing side hasn't been updated.
	{
		return false;
	}

	//int actual = sequences[currSeq].trigger;s
	/*if(next == trig::A_5SIDESWITCH && actual != trig::CMD_5)
		return false;*/

	if (sequences[currSeq].props.gotoSeq == seq) //Don't interrupt it by the next sequence, which will be shown anyway.
		return false;

	if (currentState == state::PAIN_GRND || currentState == state::PAIN_AIR) //You're in pain, what can you do?
		return false;

	//Attack cancel rules and all that stuff go here.
	GotoSequence(seq);
	return true;
}

void Character::GotoSequence(int seq)
{
	if (seq < 0)
		return;

	isKickingAss = false;
	spriteSide = side;
	currSeq = seq;
	currFrame = 0;
	GotoFrame(0);

	TransitionInto(sequences[currSeq].props.machineState);
}

void Character::GotoFrame(int frame)
{

	alreadyHit = false;
	currFrame = frame;
	framePointer = &sequences[currSeq].frames[currFrame];

	if (framePointer->frameProp.flags & flag::RESET_INFLICTED_VEL)
	{
		for (int i = HITPUSHx; i <= PUSHBACKx; ++i)
		{
			impulses[i] = 0;
		}
	}

	if (!(framePointer->frameProp.flags & flag::KEEP_VEL))
	{
		vel.x = (framePointer->frameProp.vel[0] * spriteSide);
		vel.y = (framePointer->frameProp.vel[1]);
	}
	if (!(framePointer->frameProp.flags & flag::KEEP_ACC))
	{
		accel.x = framePointer->frameProp.accel[0] * spriteSide;
		accel.y = framePointer->frameProp.accel[1];
	}
	//std::cout << sequences[currSeq].trigger << ": " << framePointer->frameProp.vel.x << "\t" << frame  << "\n";

	frameDuration = sequences[currSeq].frames[currFrame].frameProp.duration;
}

void Character::TransitionInto(int state)
{
	currentState = state;
	switch (state)
	{
	case state::GROUNDED:
	case state::BUSY_GRND:
	case state::PAIN_GRND:
		selectedTable = actTableG;
		selectedMotionList = motionListDataG;
		break;
	case state::BUSY_AIR:
	case state::PAIN_AIR:
	case state::AIRBORNE:
		selectedTable = actTableA;
		selectedMotionList = motionListDataA;
		break;
	default:
		std::cout << "default transition(!)";
	}
}

void Character::Update()
{
	switch (currentState)
	{
	case state::GROUNDED:
	case state::BUSY_GRND:
	case state::PAIN_GRND:
		UpdateGround();
		break;
	case state::BUSY_AIR:
	case state::PAIN_AIR:
	case state::AIRBORNE:
		UpdateAirborne();
		break;
	}
}

void Character::ResolveHit(int keypress) //key processing really shouldn't be here.
{
	if (gotHit)
	{
		target->hitstop = hitstop = hitTargetFrame->frameProp.hitstop;

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
		if (currentState == state::GROUNDED && (keypress & left) && !(keypress & right))
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
		}

		//TODO: Remove this nasty hack
		vel.y = hitTargetFrame->frameProp.push[1];

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

void Character::UpdateGround()
{
	if (hitstop > 0)
	{
		--hitstop;
		return;
	}

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

	for (int i = HITPUSHx; i <= PUSHBACKx; ++i)
	{
		if (impulses[i] > friction + FixedPoint(0.1))
		{
			impulses[i] -= friction;
		}
		else if (impulses[i] < -friction - FixedPoint(0.1))
		{
			impulses[i] += friction;
		}
		else
		{
			impulses[i] = 0;
		}

		Translate(impulses[i], 0);
	}

	if (touchedWall != 0)
	{
		target->root.x += -impulses[HITPUSHx];
	}

	if (framePointer->frameProp.flags & flag::FRICTION)
	{
		if (vel.x > friction + 0.1)
		{
			vel.x -= friction;
		}
		else if (vel.x < -friction - 0.1)
		{
			vel.x += friction;
		}
		else
		{
			vel.x = 0;
		}
	}

	vel.x += accel.x;

	Translate(vel.x, 0);

	--frameDuration;
	if (frameDuration == 0)
	{
		if (currFrame < sequences[currSeq].frames.size() - 1)
		{
			currFrame += 1;
			GotoFrame(currFrame);
		}
		else if (sequences[currSeq].props.loops)
			GotoFrame(sequences[currSeq].props.beginLoop);
		else //If it doesn't loop go to the next specified sequence.
			GotoSequence(sequences[currSeq].props.gotoSeq);
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

void Character::UpdateAirborne()
{
	if (hitstop > 0)
	{
		--hitstop;
		return;
	}

	if (framePointer->frameProp.flags & flag::GRAVITY) //gravity.
		vel.y -= gravity;

	if (framePointer->frameProp.flags & flag::FRICTION)
	{
		if (vel.x > friction + 0.1)
		{
			vel.x -= friction;
		}
		else if (vel.x < -friction - 0.1)
		{
			vel.x += friction;
		}
		else
		{
			vel.x = 0;
		}
	}

	for (int i = HITPUSHx; i <= PUSHBACKx; ++i)
	{
		if (impulses[i] > friction + 0.1)
		{
			impulses[i] -= friction;
		}
		else if (impulses[i] < -friction - 0.1)
		{
			impulses[i] += friction;
		}
		else
		{
			impulses[i] = 0;
		}

		root.x += impulses[i];
	}

	if (touchedWall != 0)
	{
		target->root.x += -impulses[HITPUSHx];
	}

	vel += accel;
	Translate(vel);

	if (root.y < floorPos) //Check collision with floor
	{
		vel.y = 0;
		root.y = floorPos;
		if (currentState == state::BUSY_AIR)
			GotoSequence(sequences[sequences[currSeq].props.gotoSeq].props.gotoSeq);
		else
			GotoSequence(sequences[currSeq].props.gotoSeq);
		//TransitionInto(sequences[currSeq].machineState);
	}

	--frameDuration;
	if (frameDuration == 0)
	{
		if (currFrame < sequences[currSeq].frames.size() - 1)
		{
			currFrame += 1;
			GotoFrame(currFrame);
		}
		else if (sequences[currSeq].props.loops)
			GotoFrame(sequences[currSeq].props.beginLoop);
		else //If it doesn't loop go the next specified sequence..
			GotoSequence(sequences[currSeq].props.gotoSeq);
	}
}
