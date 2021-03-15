#include <cmath>
#include <deque>
#include <fstream>
#include <string>
#include <iostream>

#include <glad/glad.h>	//These shouldn't be here but w/e for now

#include "chara.h"
#include "chara_input.h"
#include "raw_input.h" //Used only by Character::ResolveHit

bool Character::isColliding;

const uint64_t SANITY_CHECK = 0x1d150c10c001506f;
const FixedPoint floorPos(32);

Character::Character(FixedPoint xPos, float _side, std::string charFile) :
	touchedWall(0),
	health(10000),
	side(_side),
	actTableG{0},
	actTableA{0},
	currSeq(0),
	currFrame(0),
	hitstop(0),
	spriteSide(_side),

	painType(0),
	alreadyHit(false),
	isKickingAss(false),
	gotHit(0),
	gravity(0.5),
	friction(0.1)
{
	root.x = xPos;
	root.y = floorPos;

	//loads character from a file and fills sequences/frames and all that yadda.
	std::ifstream file(charFile, std::ios_base::in | std::ios_base::binary);
	if (!file.is_open())
	{
		std::cerr << "Couldn't open character file: ";
		goto errorlog;
	}

	uint64_t sanity_check;
	uint16_t header_bytes;
	CharFileHeader h;

	file.read((char *)&sanity_check, sizeof(uint64_t));
	if (sanity_check != SANITY_CHECK)
	{
		std::cerr << "This is not a proper character file: ";
		goto errorlog;
	}

	file.read((char *)&header_bytes, sizeof(uint16_t));
	file.read((char *)&h, header_bytes);

	int table_size;
	file.read((char *)&table_size, sizeof(int));
	file.read((char *)actTableG, sizeof(int) * table_size);
	file.read((char *)actTableA, sizeof(int) * table_size);

	for (int i = 0; i < 64; ++i)
	{
		actTableG[i] -= 1;
		actTableA[i] -= 1;
	}

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

	sequences.resize(h.sequences_n);
	for (uint16_t i = 0; i < h.sequences_n; ++i)
	{
		uint8_t namelength = 0;
		file.read((char *)&namelength, sizeof(uint8_t));
		file.ignore(namelength); //We proceed to skip the sequence name string since we won't use it at all.

		file.read((char *)&sequences[i].level, sizeof(int));
		file.read((char *)&sequences[i].metercost, sizeof(int));
		file.read((char *)&sequences[i].loops, sizeof(bool));
		file.read((char *)&sequences[i].beginLoop, sizeof(int));
		file.read((char *)&sequences[i].gotoSeq, sizeof(int));
		sequences[i].gotoSeq -= 1;

		file.read((char *)&sequences[i].machineState, sizeof(int));

		uint8_t seqlength;
		file.read((char *)&seqlength, sizeof(uint8_t));

		sequences[i].frames.resize(seqlength);
		sequences[i].frameNumber = seqlength;
		for (uint8_t i2 = 0; i2 < seqlength; ++i2)
		{
			file.read((char *)&sequences[i].frames[i2].imagepos, sizeof(float) * 8);

			//How many boxes are used per frame
			int16_t activeGreens;
			int16_t activeReds;
			int8_t activeCols; //as in collision box, not column.
			file.read((char *)&activeGreens, sizeof(int16_t));
			file.read((char *)&activeReds, sizeof(int16_t));
			file.read((char *)&activeCols, sizeof(int8_t));

			sequences[i].frames[i2].greenboxActive = activeGreens;
			sequences[i].frames[i2].redboxActive = activeReds;
			sequences[i].frames[i2].colboxActive = activeCols;

			if (activeGreens > 32 * 2 * 2 || activeReds > 32 * 2 * 2)
			{
				std::cerr << "Seq " << i << " frame " << i2 << " exceeds the box limit. At: " << activeGreens << " " << activeReds << " in: ";
				goto errorlog;
			}

			file.ignore(sizeof(uint16_t)); //ignores an uint16_t containing FrameProp bytes
			file.read((char *)&sequences[i].frames[i2].frameProp, sizeof(Frame_property));

			file.read((char *)sequences[i].frames[i2].greenboxes, sizeof(float) * activeGreens);
			file.read((char *)sequences[i].frames[i2].redboxes, sizeof(float) * activeReds);

			float colbox[1 * 4 * 2];
			file.read((char *)colbox, sizeof(float) * activeCols);

			sequences[i].frames[i2].colbox.bottomLeft.x = colbox[0];
			sequences[i].frames[i2].colbox.bottomLeft.y = colbox[1];
			sequences[i].frames[i2].colbox.topRight.x = colbox[4];
			sequences[i].frames[i2].colbox.topRight.y = colbox[5];

			uint16_t filepathLenght;
			file.read((char *)&filepathLenght, sizeof(uint16_t));

			if (filepathLenght > 0) //If this string exist then extract the rest and save it.
			{
				uint16_t filenameLenght;
				file.read((char *)&filenameLenght, sizeof(uint16_t));

				std::string filepath(filepathLenght, '\0');
				std::string filename(filenameLenght, '\0');

				file.read((char *)filepath.data(), sizeof(char) * filepathLenght);
				file.read((char *)filename.data(), sizeof(char) * filenameLenght);

				sequences[i].frames[i2].spriteImage = new Texture();

				std::string defaultPath("images/char/");
				defaultPath.append(filename);

				sequences[i].frames[i2].spriteImage->Load(defaultPath);
				sequences[i].frames[i2].spriteImage->Apply();
				sequences[i].frames[i2].spriteImage->Unload();
			}
		}
	}

	file.close();

	GotoSequence(actTableG[0]);
	GotoFrame(0);

	//TransitionInto(state::GROUNDED);
	return;

errorlog:
	std::cerr << charFile << "\n";
	file.close();
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

void Character::Draw()
{
	if (framePointer->spriteImage)
		glBindTexture(GL_TEXTURE_2D, framePointer->spriteImage->id);
	for (int i = 0; i < 8; i += 2)
	{
		imagepos[i] = framePointer->imagepos[i] * spriteSide + (float)(root.x); //x
		imagepos[i + 1] = framePointer->imagepos[i + 1] + (float)(root.y);		//y
	}
	glVertexPointer(2, GL_FLOAT, 0, imagepos);
	glDrawArrays(GL_QUADS, 0, 4);

	//Draws hitboxes
	/*
	for(int i = 0; i < framePointer->redboxActive; i+=2)
	{
        colpos[i] = framePointer->redboxes[i]*side+(float)root.x;	//x
        colpos[i+1] = framePointer->redboxes[i+1]+(float)root.y; //y
	}

	
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, 0);
	glColor4f(1.0,1.0,1.0, 0.6);
	glVertexPointer(2, GL_FLOAT, 0, colpos);
	glDrawArrays(GL_QUADS, 0, framePointer->redboxActive/2);
	glActiveTexture(GL_TEXTURE0);
	glColor3f(1,1,1);
	*/
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
	int greenboxN = framePointer->greenboxActive;
	int redboxN = target->framePointer->redboxActive;

	int i_s = 0;
	int i2_s = 0;

	if (side == -1)
		i_s = 2;
	if (target->side == -1)
		i2_s = 2;

	int myX = root.x;
	int myY = root.y;
	int hisX = target->root.x;
	int hisY = target->root.y;

	float A0, A1, A4, A5; //BL(x,y) TR(x,y)
	float B0, B1, B4, B5;

	bool isHit = false;
	for (int i = i_s; i < greenboxN; i += 8)
	{

		A0 = framePointer->greenboxes[i] * side + myX;
		A1 = framePointer->greenboxes[i + 1] + myY;
		A4 = framePointer->greenboxes[i + 4] * side + myX;
		A5 = framePointer->greenboxes[i + 5] + myY;

		for (int i2 = i2_s; i2 < redboxN; i2 += 8)
		{

			B0 = target->framePointer->redboxes[i2] * target->side + hisX;
			B1 = target->framePointer->redboxes[i2 + 1] + hisY;
			B4 = target->framePointer->redboxes[i2 + 4] * target->side + hisX;
			B5 = target->framePointer->redboxes[i2 + 5] + hisY;

			//std::cout << framePointer << "\t" << target->framePointer << "\n";
			//std::cout << A4 << "<"<< B0 << " "<< B4 << "<"<< A0 << " "<< A5 << "<"<< B1 << " "<< B5 << "<"<< A1<< "\n";

			if (!(A4 < B0 || B4 < A0 || A5 < B1 || B5 < A1)) //It's easier to test if they are (not) separated than checking if they're touching.
			{
				//std::cout << "hit";
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

	if (sequences[currSeq].machineState == state::BUSY_GRND) //Checks if it should ignore the next command
	{
		if (!isKickingAss && !(framePointer->frameProp.flags & flag::CANCEL_WHIFF))
			return false;
		if (!(framePointer->frameProp.flags & flag::CANCELLABLE)) //if not cancellable
			return false;
		if (!(sequences[seq].machineState == state::BUSY_GRND || sequences[seq].machineState == state::BUSY_AIR)) //if next is not attack
			return false;
		GotoSequence(seq);
		return true;
	}
	else if (sequences[currSeq].machineState == state::BUSY_AIR)
	{
		if (!isKickingAss && !(framePointer->frameProp.flags & flag::CANCEL_WHIFF))
			return false;
		if (!(framePointer->frameProp.flags & flag::CANCELLABLE)) //if not cancellable
			return false;
		if (sequences[seq].machineState != state::BUSY_AIR) //if next is not attack
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

	if (sequences[currSeq].gotoSeq == seq) //Don't interrupt it by the next sequence, which will be shown anyway.
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
	/*int cmd = sequences[seq].trigger;
	//std::cout << cmd << "\n";
	if(cmd < trig::A_8LAND || cmd > trig::A_9LAND) //Do not mirror when landing.
	{
		/*if(spriteSide != side)
		{*/
	spriteSide = side;
	/*if(cmd == trig::CMD_5)
			{
				SuggestSequence(trig2Seq[trig::A_5SIDESWITCH]);
				return;
			}
		//}
	}*/

	currSeq = seq;
	currFrame = 0;
	GotoFrame(0);

	TransitionInto(sequences[currSeq].machineState);
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
			target->impulses[PUSHBACKx] = hitTargetFrame->frameProp.pushback[1] * -target->side;
		}
		else
		{
			frameDuration = hitTargetFrame->frameProp.hitstun;
			health -= hitTargetFrame->frameProp.damage[2];
			target->impulses[PUSHBACKx] = hitTargetFrame->frameProp.pushback[0] * -target->side;
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
		if (currFrame < sequences[currSeq].frameNumber - 1)
		{
			currFrame += 1;
			GotoFrame(currFrame);
		}
		else if (sequences[currSeq].loops)
			GotoFrame(sequences[currSeq].beginLoop);
		else //If it doesn't loop go to the next specified sequence.
			GotoSequence(sequences[currSeq].gotoSeq);
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
			GotoSequence(sequences[sequences[currSeq].gotoSeq].gotoSeq);
		else
			GotoSequence(sequences[currSeq].gotoSeq);
		//TransitionInto(sequences[currSeq].machineState);
	}

	--frameDuration;
	if (frameDuration == 0)
	{
		if (currFrame < sequences[currSeq].frameNumber - 1)
		{
			currFrame += 1;
			GotoFrame(currFrame);
		}
		else if (sequences[currSeq].loops)
			GotoFrame(sequences[currSeq].beginLoop);
		else //If it doesn't loop go the next specified sequence..
			GotoSequence(sequences[currSeq].gotoSeq);
	}
}
