#include "actor.h"
#include <glm/ext/matrix_transform.hpp>

Actor::Actor(std::vector<Sequence> &sequences, sol::state &lua) :
lua(lua),
sequences(sequences)
{
}

void Actor::GotoSequence(int seq)
{
	if (seq < 0)
		seq = 0;

	currSeq = seq;
	currFrame = 0;
	totalSubframeCount = 0;
	comboType = none;
	hitCount = 0;
	attack.Clear();

	seqPointer = &sequences[currSeq];
	GotoFrame(0);
}

bool Actor::GotoFrame(int frame)
{
	//OOB frame
	if(frame >= seqPointer->frames.size() || frame < 0)
	{
		KillSelf();
		return false;
	}

	subframeCount = 0;
	currFrame = frame;
	framePointer = &seqPointer->frames[currFrame];

	if(framePointer->frameProp.loopN > 0)
		loopCounter = framePointer->frameProp.loopN;

	if(framePointer->frameProp.flags & flag::startHit)
		hitCount = 1;
	
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

	if(framePointer->hasFunction)
	{
		auto result = framePointer->frameScript(this);
		if(!result.valid())
		{
			sol::error err = result;
			std::cerr << err.what() << "\n";
		}
	}
	return true;
}

void Actor::Translate(Point2d<FixedPoint> amount)
{
	root += amount;
}

void Actor::Translate(FixedPoint x, FixedPoint y)
{
	root.x += x;
	root.y += y;
}

void Actor::SetSide(int _side)
{
	if(_side >= 0)
		side = 1;
	else
		side = -1;
}

int Actor::GetSide(){return side;}

int Actor::GetSpriteIndex()
{
	return framePointer->frameProp.spriteIndex;
}

glm::mat4 Actor::GetSpriteTransform()
{
	int x = framePointer->frameProp.spriteOffset[0];
	int y = framePointer->frameProp.spriteOffset[1];
	glm::mat4 transform = glm::scale(glm::mat4(1.f), glm::vec3(side,1,0))*framePointer->transform *
		glm::translate(glm::mat4(1.f), glm::vec3(x, -y, 0));
	return glm::translate(glm::mat4(1.f), glm::vec3(root.x, root.y, 0))*transform;
}

void Actor::SeqFun()
{
	if(seqPointer->hasFunction)
	{
		auto result = seqPointer->function(this);
		if(!result.valid())
		{
			sol::error err = result;
			std::cerr << err.what() << "\n";
		}
	}
}

void Actor::Update()
{
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
			if(framePointer->frameProp.relativeJump)
				GotoSequence(currSeq+framePointer->frameProp.jumpTo);
			else
				GotoSequence(framePointer->frameProp.jumpTo);
		}
		else
		{
			currFrame += 1;	
			if(!GotoFrame(currFrame)) //If dead don't continue;
				return;
		}
	}

	if (flags & floorCheck && root.y + vel.y < floorPos) //Check collision with floor
	{
		root.y = floorPos;
		GotoFrame(seqPointer->props.landFrame);
	}

	SeqFun();
	if (hitstop > 0)
	{
		--hitstop;
		return;
	}
	
	Translate(vel);
	vel += accel;
	if (root.y < floorPos) //Check collision with floor
	{
		root.y = floorPos;
	}

	--frameDuration;
	++totalSubframeCount;
	++subframeCount;
	
}

Frame *Actor::GetCurrentFrame()
{
	return framePointer;
}

bool Actor::HitCollision(const Actor& hurt, const Actor& hit)
{
	if(hurt.hittable && hit.hitCount > 0)
	{
		for(auto hurtbox : hurt.framePointer->greenboxes)
		{
			if(hurt.side < 0)
				hurtbox = hurtbox.FlipHorizontal();
			hurtbox = hurtbox.Translate(hurt.root);
			for(auto hitbox : hit.framePointer->redboxes)
			{
				if(hit.side < 0)
					hitbox = hitbox.FlipHorizontal();
				hitbox = hitbox.Translate(hit.root);
				if(hitbox.Intersects(hurtbox))
				{
					return true;
				}
			}
		}
	}
	return false;
}

Actor& Actor::SpawnChild(int sequence)
{
	Actor child(sequences, lua);
	child.parent = this;
	child.GotoSequence(sequence);
	children.push_back(std::move(child));
	children.back().myPos = --children.end();
	return children.back();
}

void Actor::KillSelf()
{
	if(parent)
	{
		parent->children.erase(myPos);
	}
}

void Actor::GetAllChildren(std::vector<Actor*> &list, bool includeSelf)
{
	for(auto &child : children)
	{
		child.GetAllChildren(list);
	}
	if(includeSelf)
		list.push_back(this);
}

int Actor::ResolveHit(int keypress, Actor *hitter)
{
	//Todo call hit lua func
	return hurt;
}

void Actor::DeclareActorLua(sol::state &lua)
{
	lua.new_usertype<HitDef>("HitDef",
		"attackFlags", &HitDef::attackFlags, 
		"damage", &HitDef::damage, 
		"guardDamage", &HitDef::guardDamage, 
		"correction", &HitDef::correction, 
		"correctionType", &HitDef::correctionType, 
		"meterGain", &HitDef::meterGain, 
		"hitStop", &HitDef::hitStop, 
		"blockStop", &HitDef::blockStop, 
		"untech", &HitDef::untech, 
		"blockStun", &HitDef::blockstun,
		"priority", &HitDef::priority, 
		"soundFx", &HitDef::soundFx, 
		"hitFx", &HitDef::hitFx,
		"SetVectors", &HitDef::SetVectors
	);

	lua.new_usertype<Actor>("Actor",
		"GotoFrame", &Actor::GotoFrame,
		"GotoSequence", &Actor::GotoSequence,
		"GetSide", &Actor::GetSide,
		"GetPos", [](Actor &actor){return std::make_tuple(actor.root.x.value, actor.root.y.value);},
		"SetPos", [](Actor &actor, int x, int y){actor.root.x.value = x; actor.root.y.value = y;},
		"GetVel", [](Actor &actor){return std::make_tuple(actor.vel.x.value, actor.vel.y.value);},
		"SetVel", [](Actor &actor, int x, int y){actor.vel.x.value = x; actor.vel.y.value = y;},
		"GetSide", &Actor::GetSide,
		"SetSide", &Actor::SetSide,
		"currentFrame", sol::readonly(&Actor::currFrame),
		"currentSequence", sol::readonly(&Actor::currSeq),
		"subframeCount", sol::readonly(&Actor::subframeCount),
		"totalSubframeCount", sol::readonly(&Actor::totalSubframeCount),
		"SpawnChild", &Actor::SpawnChild,
		"KillSelf", &Actor::KillSelf,
		"ChildCount", [](Actor &actor){return actor.children.size();},
		"hitDef", &Actor::attack,
		"hittable", &Actor::hittable,
		"comboType", sol::readonly(&Actor::comboType),
		"userData", &Actor::userData,
		"flags", &Actor::flags
	);
}

void HitDef::SetVectors(int state, sol::table onHitTbl, sol::table onBlockTbl)
{
	sol::table *luaTables[2] = {&onHitTbl, &onBlockTbl};
	for(int i = 0; i < 2; i++)
	{
		VectorTable &vector = vectorTables[state][i];
		vector.maxPushBackTime = (*luaTables[i])["maxTime"].get_or(0x7FFFFFFF);
		vector.xSpeed = (*luaTables[i])["xSpeed"].get_or(0);
		vector.ySpeed = (*luaTables[i])["ySpeed"].get_or(0);
		vector.xAccel = (*luaTables[i])["xAccel"].get_or(0);
		vector.yAccel = (*luaTables[i])["yAccel"].get_or(0);
		vector.sequenceName = (*luaTables[i])["ani"].get_or(std::string());
	}
}

void HitDef::Clear()
{
	attackFlags = 0;
	damage = 0;
	guardDamage = 0;
	correction = 0;
	correctionType = 0;
	meterGain = 0;
	hitStop = 0;
	blockStop = -1;
	untech = 0;
	blockstun = 0; //Untech and block
	priority = 0;
	soundFx = 0;
	hitFx = 0;
	vectorTables.clear();
}