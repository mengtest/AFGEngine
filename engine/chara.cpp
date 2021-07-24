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
	player(sequences),
	touchedWall(0)
{
	player.root.x = xPos;
	player.root.y = floorPos;
	player.SetSide(side);

	//loads character from a file and fills sequences/frames and all that yadda.
	ScriptSetup();
	cmd.LoadFromLua("data/char/vaki/moves.lua", lua);
	LoadSequences(sequences, charFile, lua); //Sequences refer to script.

	player.GotoSequence(0);
	player.GotoFrame(0);
	return;
}

Point2d<FixedPoint> Character::getXYCoords()
{
	return player.root;
}

void Character::SetCameraRef(Camera *ref)
{
	currView = ref;
}

void Character::setTarget(Character *t)
{
	target = t;
}


//TODO: Remove these dudes
int Character::GetSpriteIndex()
{
	return player.GetSpriteIndex();
}
glm::mat4 Character::GetSpriteTransform()
{
	return player.GetSpriteTransform();
}


void Character::Collision(Character *playerOne, Character *playerTwo)
{
	isColliding = false;
	Actor *actor1 = &playerOne->player;
	Actor *actor2 = &playerTwo->player;

	Rect2d<FixedPoint> colBlue = actor1->framePointer->colbox;
	Rect2d<FixedPoint> colRed = actor2->framePointer->colbox;

	if (actor1->side == -1)
		colBlue = colBlue.FlipHorizontal();
	if (actor2->side == -1)
		colRed = colRed.FlipHorizontal();

	colBlue = colBlue.Translate(actor1->root);
	colRed = colRed.Translate(actor2->root);

	isColliding = colBlue.Intersects(colRed);

	if (isColliding)
	{
		const FixedPoint magic(480); //Stage width or large number?

		FixedPoint getAway(0,5);
		FixedPoint getAway1, getAway2;

		if (playerOne->touchedWall != 0 || playerTwo->touchedWall != 0)
			getAway = 1;

		if (actor1->root.x + playerOne->touchedWall * magic < actor2->root.x + playerTwo->touchedWall * magic)
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
	if (Actor::HitCollision(player, target->player))
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

	if (player.root.x <= currView->GetWallPos(camera::leftWall) + wallOffset)
	{
		touchedWall = -1;
		player.root.x = currView->GetWallPos(camera::leftWall) + wallOffset;
	}

	else if (player.root.x >= currView->GetWallPos(camera::rightWall) - wallOffset)
	{
		touchedWall = 1;
		player.root.x = currView->GetWallPos(camera::rightWall) - wallOffset;
	}

	if (touchedWall == target->touchedWall) //Someone already has the wall.
		touchedWall = 0;
}

void Character::Translate(Point2d<FixedPoint> amount)
{
	player.Translate(amount);
	BoundaryCollision();
}

void Character::Translate(FixedPoint x, FixedPoint y)
{
	player.Translate(x,y);
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
	if(mustTurnAround)
	{
		mustTurnAround = false;
		player.SetSide(-player.GetSide());
	}

	interrumpible = false;
	comboFlag = false;
	
	player.GotoSequence(seq);
}

void Character::GotoFrame(int frame)
{
	if(!player.GotoFrame(frame))
	{
		//Destroy?
		mustTurnAround = false;
		GotoSequence(0);
		return;
	}

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
		if (player.GetSide() < 0) //Inverts absolute input depending on side. Apparent input is corrected.
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
	if (player.hitstop > 0)
	{
		--player.hitstop;
		player.SeqFun();
		return;
	}

	if (touchedWall != 0)
	{
		target->player.root.x += -impulses[0];
	}

	player.vel += player.accel;
	Translate(player.vel);

	if (player.root.y < floorPos) //Check collision with floor
	{
		player.root.y = floorPos;
		//Jump to landing frame.
		GotoFrame(player.seqPointer->props.landFrame);
	}
	if((player.framePointer->frameProp.state == state::stand || player.framePointer->frameProp.state == state::crouch) &&
		(player.root.x < target->player.root.x && player.side == -1 || player.root.x > target->player.root.x && player.side == 1))
		mustTurnAround = true;

	--player.frameDuration;
	++player.totalSubframeCount;
	++player.subframeCount;
	if (player.frameDuration == 0)
	{
		int jump = player.framePointer->frameProp.jumpType;
		if(jump == jump::frame)
		{
			if(player.framePointer->frameProp.relativeJump)
				player.currFrame += player.framePointer->frameProp.jumpTo;
			else
				player.currFrame = player.framePointer->frameProp.jumpTo;
		}
		else if(jump == jump::loop)
		{
			if(player.loopCounter > 1)
			{
				if(player.framePointer->frameProp.relativeJump)
					player.currFrame += player.framePointer->frameProp.jumpTo;
				else
					player.currFrame = player.framePointer->frameProp.jumpTo;
				player.loopCounter--;
			}
			player.currFrame++;
		}
		else if(jump == jump::seq)
		{
			mustTurnAround = false;
			if(player.framePointer->frameProp.relativeJump)
				GotoSequence(player.currSeq+player.framePointer->frameProp.jumpTo);
			else
				GotoSequence(player.framePointer->frameProp.jumpTo);
		}
		else
			player.currFrame += 1;
			
		GotoFrame(player.currFrame);
	}
	
	player.SeqFun();
}

bool Character::TurnAround(int sequence)
{
	if (player.root.x < target->player.root.x && player.GetSide() == -1) //Side switching.
	{
		GotoSequence(sequence);
		player.SetSide(1);
		return true;
	}
	else if (player.root.x > target->player.root.x && player.GetSide() == 1)
	{
		GotoSequence(sequence);
		player.SetSide(-1);
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

	int inputSide = player.GetSide();
	if(mustTurnAround)
		inputSide = -inputSide;
	
	MotionData command;
	if(player.GetCurrentFrame()->frameProp.state == state::air)
		command = cmd.ProcessInput(keyPresses, "air", inputSide);
	else
		command = cmd.ProcessInput(keyPresses, "ground", inputSide);
	
	if(!(player.GetCurrentFrame()->frameProp.flags & flag::canMove) &&
		(!interrumpible || !(command.flags & CommandInputs::interrupts) || player.totalSubframeCount > command.bufLen))
		return;

	//Don't transition to the seq if the command is marked as a neutral move (walking).
	if(command.flags & CommandInputs::neutralMove && player.GetCurrentFrame()->frameProp.flags & flag::dontWalk)
		return;

	//The sequence can't go to itself unless it's flagged as such
	if(!(command.flags & CommandInputs::repeatable) && player.currSeq == command.seqRef) 
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

	auto actor = lua["actor"].get_or_create<sol::table>();
	actor.set("multiplier", speedMultiplier);
	actor.set_function("getSide", [this](){return player.side;});
	actor.set_function("getPos", [this](){return std::make_tuple(player.root.x.value, player.root.y.value);});
	actor.set_function("setPos", [this](int x, int y){player.root.x.value = x; player.root.y.value = y;});
	actor.set_function("getVel", [this](){return std::make_tuple(player.vel.x.value, player.vel.y.value);});
	actor.set_function("setVel", [this](int x, int y){player.vel.x.value = x; player.vel.y.value = y;});
	actor.set_function("currentFrame", [this](){return player.currFrame;});
	actor.set_function("currentSequence", [this](){return player.currSeq;});
	actor.set_function("gotoFrame", &Character::GotoFrame, this);
	actor.set_function("gotoSequence", &Character::GotoSequence, this);
	actor.set_function("turnAround", &Character::TurnAround, this);
	actor.set_function("getInput", [this]() -> unsigned int{return this->lastKey;});
	actor.set_function("getInputRelative", [this]() -> unsigned int{
		if(player.GetSide() > 0)
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