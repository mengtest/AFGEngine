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
Actor(sequences, lua),
touchedWall(0)
{
	root.x = xPos;
	root.y = floorPos;
	SetSide(side);
	hittable = true;

	//loads character from a file and fills sequences/frames and all that yadda.
	if(!ScriptSetup())
		abort();
	cmd.LoadFromLua("data/char/vaki/moves.lua", lua);
	LoadSequences(sequences, charFile, lua); //Sequences refer to script.

	GotoSequence(0);
	GotoFrame(0);

	sol::protected_function init = lua["_init"];
	if(init.get_type() == sol::type::function)
		init();

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

void Character::HitCollision(Character &blue, Character &red, int blueKey, int redKey)
{
	std::list<Actor*> blueList, redList;
	//Idea: getEvil/GoodChildren maybe? lol
	blue.GetAllChildren(blueList); 
	red.GetAllChildren(redList); 
	for(auto blue : blueList)
	{
		for(auto red : redList)
		{
			//blue hits red
			if (Actor::HitCollision(*red, *blue))
			{
				blue->comboType = red->ResolveHit(redKey, blue);
				if(blue->comboType == blocked)
				{
					if(blue->attack.blockStop >= 0)
						blue->hitstop = blue->attack.blockStop;
					else
						blue->hitstop = blue->attack.hitStop;
				}
				else if(blue->comboType == hurt)
					blue->hitstop = blue->attack.hitStop;
				blue->hitCount--;
			}
			//red hits blue
			if (Actor::HitCollision(*blue, *red))
			{
				red->comboType = blue->ResolveHit(blueKey, red);
				if(red->comboType == blocked)
				{
					if(red->attack.blockStop >= 0)
						red->hitstop = red->attack.blockStop;
					else
						red->hitstop = red->attack.hitStop;
				}
				else if(red->comboType == hurt)
					red->hitstop = red->attack.hitStop;
				red->hitCount--;
			}
		}
	}
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


int Character::ResolveHit(int keypress, Actor *hitter) //key processing really shouldn't be here.
{
	HitDef *hitData = &hitter->attack;
	keypress = SanitizeKey(keypress);
	gotHit = true;
	hitstop = hitData->hitStop;

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
	//Can block
	if (blockFlag || (framePointer->frameProp.flags & flag::canMove && keypress & left))
	{
		//Always for now.
		if(hitData->blockStop>=0)
			hitstop = hitData->blockStop;
		blocked	= true;
		blockFlag = true;
		blockTime = hitData->blockstun;
	}
	else
		blockFlag = false;

	int state = framePointer->frameProp.state;
	if(hitData->vectorTables.count(state) == 0)
		state = 0; //TODO: Fallback state from lua?
	if(hitData->vectorTables.count(state) > 0)
	{
		auto &vt = hitData->vectorTables[state][blocked];
		int seq = lua["_seqTable"][vt.sequenceName].get_or(-1);
		if(seq > 0)
		{
			hurtSeq = seq;
			vel.x.value = vt.xSpeed*speedMultiplier*hitter->side;
			vel.y.value = vt.ySpeed*speedMultiplier;
			accel.x.value = vt.xAccel*speedMultiplier*hitter->side;
			accel.y.value = vt.yAccel*speedMultiplier;
			pushTimer = vt.maxPushBackTime;	
			friction = true;
		}
	}

	if (blocked)
		return hitType::blocked;
	else
	{
		health -= hitData->damage;
		return hitType::hurt;
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
		return;

	
	if(mustTurnAround)
	{
		mustTurnAround = false;
		side = -side;
	}
	
	interrumpible = false;
	comboType = none;
	currSeq = seq;
	currFrame = 0;
	totalSubframeCount = 0;
	attack.Clear();
	hitCount = 0;

	seqPointer = &sequences[currSeq];
	GotoFrame(0);
	if(framePointer->frameProp.flags & flag::canMove)
	{
		friction = false;
		blockFlag = false;
		pushTimer = 0;
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
	
	if(gotHit) //Got hit.
	{
		GotoSequence(hurtSeq);
		SeqFun();
		hurtSeq = -1;
		gotHit = false;
	}
	else
		SeqFun();

	if (hitstop > 0)
	{
		//Shake effect
		--hitstop;
		return;
	}

	if(friction && (vel.x.value < 0 && accel.x.value < 0)) //Pushback can't accelerate. Only slow down.
	{
		vel.x.value = 0;
		accel.x.value = 0;
	}
	else if(friction && (vel.x.value > 0 && accel.x.value > 0))
	{
		vel.x.value = 0;
		accel.x.value = 0;
	}

	if (touchedWall != 0 && (pushTimer > 0))
	{
		target->root.x -= vel.x;
	}
	
	Translate(vel);
	vel += accel;

	if (root.y < floorPos) //Check collision with floor
	{
		root.y = floorPos;
		GotoFrame(seqPointer->props.landFrame);
		SeqFun(); //Extra state transition, so we run it again.
	}

	if((framePointer->frameProp.state == state::stand || framePointer->frameProp.state == state::crouch) &&
		(root.x < target->root.x && side == -1 || root.x > target->root.x && side == 1))
		mustTurnAround = true;

	if(blockTime > 0)
	{
		--blockTime;
		return;
	}

	--pushTimer;
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
			else
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
}

bool Character::TurnAround(int sequence)
{
	if (root.x < target->root.x && GetSide() < 0) //Side switching.
	{
		SetSide(1);
		GotoSequence(sequence);
		return true;
	}
	else if (root.x > target->root.x && GetSide() > 0)
	{
		SetSide(-1);
		GotoSequence(sequence);
		return true;
	}
	return false;
}

void Character::Input(input_deque &keyPresses)
{
	lastKey[0] = keyPresses[0];
	lastKey[1] = keyPresses[1];
	cmd.Charge(keyPresses);
	
	int inputSide = GetSide();
	if(mustTurnAround)
		inputSide = -inputSide;

	MotionData command;
	auto &prop = framePointer->frameProp;
	auto flags = prop.flags;
	if(framePointer->frameProp.state == state::air)
		command = cmd.ProcessInput(keyPresses, "air", inputSide);
	else
		command = cmd.ProcessInput(keyPresses, "ground", inputSide);

	if(hitstop)
	{ 
		//TODO: Keep higher priority command
		if(command.seqRef > 0 && command.priority < lastCommand.priority)
			lastCommand = command;
		return;
	}
	else
	{
		if(lastCommand.seqRef > 0)
			command = lastCommand;
		lastCommand = {};
	}
	
	if(prop.cancelType[0] > 0 && comboType != none && !(command.flags & (CommandInputs::neutralMove | CommandInputs::noCombo)))
		goto canDo;
	if(!(flags & flag::canMove) && (!interrumpible || !(command.flags & CommandInputs::interrupts) || totalSubframeCount > command.bufLen))
		return;
	canDo:

	//Don't transition to the seq if the command is marked as a neutral move (walking).
	if(command.flags & CommandInputs::neutralMove && flags & flag::dontWalk)
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

bool Character::ScriptSetup()
{
	lua.open_libraries(sol::lib::base);

	Actor::DeclareActorLua(lua);
	

	auto constant = lua["constant"].get_or_create<sol::table>();
	constant["multiplier"] = speedMultiplier;
	auto key = constant["key"].get_or_create<sol::table>();
	key["up"] = key::buf::UP;
	key["down"] = key::buf::DOWN;
	key["left"] = key::buf::LEFT;
	key["right"] = key::buf::RIGHT;
	key["any"] = key::buf::UP | key::buf::DOWN | key::buf::LEFT | key::buf::RIGHT;

	auto global = lua["global"].get_or_create<sol::table>();
	global.set_function("GetBlockTime", [this](){return blockTime;});
	global.set_function("SetBlockTime", [this](int time){blockTime=time;});
	global.set_function("TurnAround", &Character::TurnAround, this);
	global.set_function("GetInput", [this]() -> unsigned int{return this->lastKey[0];});
	global.set_function("GetInputPrev", [this]() -> unsigned int{return this->lastKey[1];});
	global.set_function("GetInputRelative", [this]() -> unsigned int{
		int lastkey = lastKey[0];
		if(GetSide() > 0)
			return lastkey;
		else{
			int right = lastkey & key::buf::RIGHT;
			int left = lastkey & key::buf::LEFT;
			return (lastkey & ~0xC) | ((!!right) << key::LEFT) | ((!!left) << key::RIGHT);
		}
	});
	auto result = lua.script_file("data/char/vaki/script.lua");
	if(!result.valid()){
		sol::error err = result;
		std::cerr << "The code has failed to run!\n"
		          << err.what() << "\nPanicking and exiting..."
		          << std::endl;
		return false;
	}

	updateFunction = lua["_update"];
	hasUpdateFunction = updateFunction.get_type() == sol::type::function;
	lua["player"] = (Actor*)this;
	return true;
}