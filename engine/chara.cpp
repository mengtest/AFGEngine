#include <cmath>
#include <deque>
#include <fstream>
#include <string>
#include <iostream>

#include <glad/glad.h>	//Transform

#include "chara.h"
#include "raw_input.h" //Used only by Character::ResolveHit

bool Character::isColliding;
const FixedPoint floorPos(32);
constexpr int speedMultiplier = 240;

Character::Character(FixedPoint xPos, float _side, std::string charFile) :
	side(_side),
	touchedWall(0)
{
	root.x = xPos;
	root.y = floorPos;

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
	glm::mat4 transform(1.f);
	float rotY = framePointer->frameProp.rotation[1];
	
	constexpr float tau = glm::pi<float>()*2.f;
	//transform = glm::scale(transform, glm::vec3(scale, scale, 1.f));
	//transform = glm::translate(transform, glm::vec3(x,y,0.f));
	//transform = glm::scale(transform, glm::vec3(scaleX,scaleY,0));
	//transform = glm::rotate(transform, rotZ*tau, glm::vec3(0.0, 0.f, 1.f));
	
	//transform = glm::rotate(transform, rotX*tau, glm::vec3(1.0, 0.f, 0.f));

	
	
	//-128.f, -40.f
	
	
	transform = glm::translate(transform, glm::vec3((root.x), root.y, 0));
	transform = glm::rotate(transform, rotY*tau, glm::vec3(0.0, 1.f, 0.f));
	transform = glm::scale(transform, glm::vec3(side, 1.f, 1.f));
	transform = glm::translate(transform, glm::vec3(-128, -40.f, 0));
	
	
	return transform;
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
		const FixedPoint magic(4800);

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

		target->comboFlag = true;
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
	comboFlag = false;
	currSeq = seq;
	currFrame = 0;
	totalSubframeCount = 0;

	seqPointer = &sequences[currSeq];
	GotoFrame(0);
}

void Character::GotoFrame(int frame)
{
	if(frame >= seqPointer->frames.size())
	{
		mustTurnAround = false;
		GotoSequence(0);
		return;
	}

	subframeCount = 0;
	alreadyHit = false;
	currFrame = frame;
	framePointer = &seqPointer->frames[currFrame];

	if(framePointer->frameProp.loopN > 0)
		loopCounter = framePointer->frameProp.loopN;

	//Keep?
	/* if (framePointer->frameProp.flags & flag::RESET_INFLICTED_VEL)
	{
		for (int i = 0; i <= 1; ++i)
		{
			impulses[i] = 0;
		}
	} */

	int *spd[2] = {&vel.x.value, &vel.y.value};
	int *acc[2] = {&accel.x.value, &accel.y.value};
	
	for(int i = 0; i < 2; i++)
	{
		int sside = 1;
		if(i == 0)
			sside = side;
		switch(framePointer->frameProp.movementType[i])
		{
		case 1:
			*spd[i] = framePointer->frameProp.vel[i]*speedMultiplier*sside;
			*acc[i] = framePointer->frameProp.accel[i]*speedMultiplier*sside;
			break;
		case 2:
			*spd[i] += framePointer->frameProp.vel[i]*speedMultiplier*sside;
			*acc[i] = framePointer->frameProp.accel[i]*speedMultiplier*sside;
			break;
		case 3:
			*spd[i] += framePointer->frameProp.vel[i]*speedMultiplier*sside;
			*acc[i] += framePointer->frameProp.accel[i]*speedMultiplier*sside;
			break;
		}
	}

	frameDuration = framePointer->frameProp.duration;
}


void Character::ResolveHit(int keypress) //key processing really shouldn't be here.
{
	return;
	if (gotHit)
	{
		target->hitstop = hitstop = 8;//hitTargetFrame->attackProp.stop[0];

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

void Character::SeqFun()
{
	if(seqPointer->hasFunction)
	{
		auto result = seqPointer->function();
		if(!result.valid())
		{
			sol::error err = result;
			std::cerr << err.what() << "\n";
		}
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
			currFrame += 1;
			
		GotoFrame(currFrame);
	}
	
	SeqFun();
}

bool Character::TurnAround(int sequence)
{
	if (root.x < target->root.x && side == -1) //Side switching.
	{
		GotoSequence(sequence);
		side = 1;
		return true;
	}
	else if (root.x > target->root.x && side == 1)
	{
		GotoSequence(sequence);
		side = -1;
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

	int inputSide = side;
	if(mustTurnAround)
		inputSide = -side;
	
	MotionData command;
	if(framePointer->frameProp.state == state::air)
		command = cmd.ProcessInput(keyPresses, "air", inputSide);
	else
		command = cmd.ProcessInput(keyPresses, "ground", inputSide);
	
	if(!(framePointer->frameProp.flags & flag::canMove) &&
		(!interrumpible || !(command.flags & CommandInputs::interrupts) || totalSubframeCount > command.bufLen))
		return;

	//Don't transition to the seq if the command is marked as a neutral move (walking).
	if(command.flags & CommandInputs::neutralMove && framePointer->frameProp.flags & flag::dontWalk)
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

	auto actor = lua["actor"].get_or_create<sol::table>();
	actor.set("multiplier", speedMultiplier);
	actor.set_function("getSide", [this](){return this->side;});
	actor.set_function("getPos", [this](){return std::make_tuple(this->root.x.value, this->root.y.value);});
	actor.set_function("setPos", [this](int x, int y){this->root.x.value = x; this->root.y.value = y;});
	actor.set_function("getVel", [this](){return std::make_tuple(this->vel.x.value, this->vel.y.value);});
	actor.set_function("setVel", [this](int x, int y){this->vel.x.value = x; this->vel.y.value = y;});
	actor.set_function("currentFrame", [this](){return this->currFrame;});
	actor.set_function("currentSequence", [this](){return this->currSeq;});
	actor.set_function("gotoFrame", &Character::GotoFrame, this);
	actor.set_function("gotoSequence", &Character::GotoSequence, this);
	actor.set_function("turnAround", &Character::TurnAround, this);
	actor.set_function("getInput", [this]() -> unsigned int{return this->lastKey;});
	actor.set_function("getInputRelative", [this]() -> unsigned int{
		if(side == 1)
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