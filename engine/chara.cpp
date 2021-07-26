#include <cmath>
#include <deque>
#include <fstream>
#include <string>
#include <iostream>

#include <glad/glad.h>	//Transform

#include "chara.h"
#include "raw_input.h" //Used only by Character::ResolveHit

bool Character::isColliding;

Character::Character(FixedPoint xPos, float side, std::string charFile) :
Actor(sequences),
touchedWall(0)
{
	root.x = xPos;
	root.y = floorPos;
	SetSide(side);

	//loads character from a file and fills sequences/frames and all that yadda.
	ScriptSetup();
	cmd.LoadFromLua("data/char/vaki/moves.lua", lua);
	LoadSequences(sequences, charFile, lua); //Sequences refer to script.

	GotoSequence(0);
	GotoFrame(0);
	return;
}

Point2d<FixedPoint> Character::getXYCoords()
{
	return root;
}

void Character::SetCameraRef(Camera *ref)
{
	currView = ref;
}

void Character::setTarget(Character *t)
{
	target = t;
}

void Character::Collision(Character *playerOne, Character *playerTwo)
{
	isColliding = false;

	Rect2d<FixedPoint> colBlue = playerOne->framePointer->colbox;
	Rect2d<FixedPoint> colRed = playerTwo->framePointer->colbox;

	if (playerOne->GetSide() < 0)
		colBlue = colBlue.FlipHorizontal();
	if (playerTwo->GetSide() < 0)
		colRed = colRed.FlipHorizontal();

	colBlue = colBlue.Translate(playerOne->root);
	colRed = colRed.Translate(playerTwo->root);

	isColliding = colBlue.Intersects(colRed);

	if (isColliding)
	{
		const FixedPoint magic(480); //Stage width or large number?

		FixedPoint getAway(0,5);
		FixedPoint getAway1, getAway2;

		if (playerOne->touchedWall != 0 || playerTwo->touchedWall != 0)
			getAway = 1;

		if (playerOne->root.x + playerOne->touchedWall * magic < playerTwo->root.x + playerTwo->touchedWall * magic)
		{
			getAway = getAway * (colBlue.topRight.x - colRed.bottomLeft.x);

			getAway1 = -getAway;
			getAway2 = getAway;
		}
		else
		{
			getAway = getAway * (colRed.topRight.x - colBlue.bottomLeft.x);

			getAway1 = getAway;
			getAway2 = -getAway;
		}

		playerOne->Translate(getAway1, 0);
		playerTwo->Translate(getAway2, 0);
	}

	return;
}

void Character::HitCollision()
{
	if (Actor::HitCollision(*this, *target))
	{
		if (target->alreadyHit) //Avoids the same frame hitting multiple times.
			return;

		target->comboFlag = true;
		target->alreadyHit = true;
		gotHit = true;
		//hitTargetFrame = target->framePointer;
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
		
	//If you're in pain. Do nothing?

	//Attack cancel rules and all that stuff go here.
	GotoSequence(seq);
	return true;
}

void Character::GotoSequence(int seq)
{
	if (seq < 0)
		seq = 0;

	
	if(mustTurnAround)
	{
		mustTurnAround = false;
		side = -side;
	}
	

	interrumpible = false;
	comboFlag = false;
	currSeq = seq;
	currFrame = 0;
	totalSubframeCount = 0;

	seqPointer = &sequences[currSeq];
	GotoFrame(0);
}

void Character::GotoFrame(int frame)
{
	if(!Actor::GotoFrame(frame))
		return;

	alreadyHit = false; //combo stuff
}


void Character::ResolveHit(int keypress) //key processing really shouldn't be here.
{
	return;
	if (gotHit)
	{
		//target->hitstop = hitstop = 8;//hitTargetFrame->attackProp.stop[0];

		int left;
		int right;
		if (GetSide() < 0) //Inverts absolute input depending on side. Apparent input is corrected.
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
				if (framePointer->frameProp.state == state::CROUCHED)
					GotoSequence(selectedTable[act::GUARD1]);
				else
					GotoSequence(selectedTable[act::GUARD4]);
				frameDuration = hitTargetFrame->frameProp.blockstun;
				blocked = true;
			}
		}

		if (!blocked)
		{
			if (framePointer->frameProp.state == state::AIRBORNE)
			{
				if (hitTargetFrame->frameProp.painType == pain::HIGH)
					GotoSequence(actTableA[act::HIGH_PAIN]);
				else
					GotoSequence(actTableA[act::MID_PAIN]);
			}
			else if (framePointer->frameProp.state != state::OTG)
			{
				if (framePointer->frameProp.state == state::CROUCHED)
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

void Character::Update()
{
	if(hasUpdateFunction)
	{
		auto result = updateFunction();
		if(!result.valid())
		{
			sol::error err = result;
			std::cerr << err.what() << "\n";
		}
	}
	if (hitstop > 0)
	{
		--hitstop;
		SeqFun();
		return;
	}

	if (touchedWall != 0)
	{
		target->root.x += -impulses[0];
	}

	vel += accel;
	Translate(vel);

	if (root.y < floorPos) //Check collision with floor
	{
		root.y = floorPos;
		//Jump to landing frame.
		GotoFrame(seqPointer->props.landFrame);
	}

	if((framePointer->frameProp.state == state::stand || framePointer->frameProp.state == state::crouch) &&
		(root.x < target->root.x && side == -1 || root.x > target->root.x && side == 1))
		mustTurnAround = true;

	--frameDuration;
	++totalSubframeCount;
	++subframeCount;
	if (frameDuration == 0)
	{
		int jump = framePointer->frameProp.jumpType;
		if(jump == jump::frame)
		{
			if(framePointer->frameProp.relativeJump)
				currFrame += framePointer->frameProp.jumpTo;
			else
				currFrame = framePointer->frameProp.jumpTo;
			GotoFrame(currFrame);
		}
		else if(jump == jump::loop)
		{
			if(loopCounter > 1)
			{
				if(framePointer->frameProp.relativeJump)
					currFrame += framePointer->frameProp.jumpTo;
				else
					currFrame = framePointer->frameProp.jumpTo;
				loopCounter--;
			}
			currFrame++;
			GotoFrame(currFrame);
		}
		else if(jump == jump::seq)
		{
			mustTurnAround = false;
			if(framePointer->frameProp.relativeJump)
				GotoSequence(currSeq+framePointer->frameProp.jumpTo);
			else
				GotoSequence(framePointer->frameProp.jumpTo);
		}
		else
		{
			currFrame += 1;	
			GotoFrame(currFrame);
		}
	}
	
	SeqFun();
}

bool Character::TurnAround(int sequence)
{
	if (root.x < target->root.x && GetSide() < 0) //Side switching.
	{
		GotoSequence(sequence);
		SetSide(1);
		return true;
	}
	else if (root.x > target->root.x && GetSide() > 0)
	{
		GotoSequence(sequence);
		SetSide(-1);
		return true;
	}
	return false;
}

void Character::Input(input_deque &keyPresses)
{
	lastKey = keyPresses.front();
	cmd.Charge(keyPresses);
	
	//Block hit
	ResolveHit((keyPresses)[0]);

	int inputSide = GetSide();
	if(mustTurnAround)
		inputSide = -inputSide;
	
	MotionData command;
	if(GetCurrentFrame()->frameProp.state == state::air)
		command = cmd.ProcessInput(keyPresses, "air", inputSide);
	else
		command = cmd.ProcessInput(keyPresses, "ground", inputSide);
	
	if(!(GetCurrentFrame()->frameProp.flags & flag::canMove) &&
		(!interrumpible || !(command.flags & CommandInputs::interrupts) || totalSubframeCount > command.bufLen))
		return;

	//Don't transition to the seq if the command is marked as a neutral move (walking).
	if(command.flags & CommandInputs::neutralMove && GetCurrentFrame()->frameProp.flags & flag::dontWalk)
		return;

	//The sequence can't go to itself unless it's flagged as such
	if(!(command.flags & CommandInputs::repeatable) && currSeq == command.seqRef) 
		return;
	
	if(command.hasCondition)
	{
		auto result = command.condition();
		if(!result.valid())
		{
			sol::error err = result;
			std::cerr << err.what() << "\n";
		}
		else if(!result.get<bool>())
			return;
	}

	if(SuggestSequence(command.seqRef))
	{
		if(command.flags & CommandInputs::wipeBuffer) 
			keyPresses.front() |= key::buf::CUT;

		if(command.flags & CommandInputs::interrumpible) 
			interrumpible = true;
	}	
}

void Character::ScriptSetup()
{
	lua.open_libraries(sol::lib::base);

	Actor::DeclareActorLua(lua);
	lua["player"] = (Actor*)this;

	auto constant = lua["constant"].get_or_create<sol::table>();
	constant["multiplier"] = speedMultiplier;
	auto key = constant["key"].get_or_create<sol::table>();
	key["up"] = key::buf::UP;
	key["down"] = key::buf::DOWN;
	key["left"] = key::buf::LEFT;
	key["right"] = key::buf::RIGHT;
	key["any"] = key::buf::UP | key::buf::DOWN | key::buf::LEFT | key::buf::RIGHT;

	auto global = lua["global"].get_or_create<sol::table>();
	global.set_function("TurnAround", &Character::TurnAround, this);
	global.set_function("GetInput", [this]() -> unsigned int{return this->lastKey;});
	global.set_function("GetInputRelative", [this]() -> unsigned int{
		if(GetSide() > 0)
			return this->lastKey;
		else{
			int right = lastKey & key::buf::RIGHT;
			int left = lastKey & key::buf::LEFT;
			return (lastKey & ~0xC) | ((!!right) << key::LEFT) | ((!!left) << key::RIGHT);
		}
	});
	auto result = lua.script_file("data/char/vaki/script.lua");
	if(!result.valid()){
		sol::error err = result;
		std::cerr << "The code has failed to run!\n"
		          << err.what() << "\nPanicking and exiting..."
		          << std::endl;
		return;
	}

	updateFunction = lua["_update"];
	hasUpdateFunction = updateFunction.get_type() == sol::type::function;
}