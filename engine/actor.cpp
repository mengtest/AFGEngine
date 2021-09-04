#include "actor.h"
#include <glm/ext/matrix_transform.hpp>

Actor::Actor(std::vector<Sequence> &sequences, sol::state &lua, std::vector<Actor> &actorList) :
lua(lua),
sequences(sequences),
actorList(actorList)
{
	//userData = lua.create_table();
}

Actor::~Actor()
{
	//userData = nullptr;
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

	seqPointer = &sequences.get()[currSeq];
	landingFrame = seqPointer->props.landFrame;
	GotoFrame(0);
}

bool Actor::GotoFrame(int frame)
{
	//OOB frame
	if(frame >= seqPointer->frames.size() || frame < 0)
		return false; //Kill self

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
			std::cerr << "Sequence("<<currSeq<<"), frame("<<currFrame<<"):\n";
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
	if(hitstop && shaking)
		x -= 2*(hitstop%2);
	glm::mat4 transform = glm::scale(glm::mat4(1.f), glm::vec3(side,1,0))*framePointer->transform*
		glm::translate(glm::mat4(1.f), glm::vec3(x, -y, 0));
	return glm::translate(glm::mat4(1.f), glm::vec3(root.x, root.y, 0))*transform*customTransform;
}

RenderOptions Actor::GetRenderOptions()
{
	auto &fp = framePointer->frameProp;
	return {
		fp.blendType,
		paletteIndex,
		fp.color[0],
		fp.color[1],
		fp.color[2],
		fp.color[3],
	};
}

void Actor::SendHitboxData(HitboxRenderer &hr)
{
	static std::vector<float> vertices;
	Frame::boxes_t *selector[] = {&framePointer->redboxes, &framePointer->greenboxes};
	for(int i = 0; i < 2; ++i)
	{
		vertices.resize(selector[i]->size()*4);
		for(int bi = 0; bi < selector[i]->size(); ++bi)
		{
			auto box = (*selector[i])[bi];
			if(side == -1)
				box = box.FlipHorizontal();
			box = box.Translate(root);
			vertices[bi*4 + 0] = box.bottomLeft.x;
			vertices[bi*4 + 1] = box.bottomLeft.y;
			vertices[bi*4 + 2] = box.topRight.x;
			vertices[bi*4 + 3] = box.topRight.y;
		}
		hr.GenerateHitboxVertices(vertices, 2-i);
	}

	auto col = framePointer->colbox;
	if(side == -1)
		col = col.FlipHorizontal();
	col = col.Translate(root);
	vertices = {col.bottomLeft.x, col.bottomLeft.y, col.topRight.x, col.topRight.y};
	hr.GenerateHitboxVertices(vertices, HitboxRenderer::gray);
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

bool Actor::AdvanceFrame()
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
				return false;
		}
	}
	return true;
}

bool Actor::Update()
{
	pastRoot = root;
	if(attachPoint)
	{
		root += attachPoint->root - attachPoint->pastRoot;
	}
	if(frozen)
		return true;
	
	if(!AdvanceFrame()) //Died
		return false;

	if (flags & floorCheck && root.y + vel.y < floorPos) //Check collision with floor
	{
		root.y = floorPos;
		GotoFrame(landingFrame);
	}

	SeqFun();
	if (hitstop > 0)
	{
		--hitstop;
		return true;
	}
	else
		shaking = false;

	if(friction) //Pushback can't accelerate. Only slow down.
	{
		if(vel.x.value < 0 && accel.x.value < 0)
		{
			vel.x.value = 0;
			accel.x.value = 0;
		}
		else if(friction && (vel.x.value > 0 && accel.x.value > 0))
		{
			vel.x.value = 0;
			accel.x.value = 0;
		}
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

	return true;	
}

Frame *Actor::GetCurrentFrame()
{
	return framePointer;
}

std::pair<bool, Point2d<FixedPoint>> Actor::HitCollision(const Actor& hurt, const Actor& hit)
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
					return {true, hitbox.MiddlePoint(hurtbox)};
				}
			}
		}
	}
	return {false,{}};
}

Actor& Actor::SpawnChild(int sequence)
{
	Actor child(sequences, lua, actorList);
	child.paletteIndex = paletteIndex;
	child.GotoSequence(sequence);
	actorList.get().push_back(std::move(child));
	return actorList.get().back();
}

int Actor::ResolveHit(int keypress, Actor *hitter)
{
	//Todo call hit lua func
	return none;
}

bool Actor::ThrowCheck(Actor& enemy, int frontRange, int upRange, int downRange)
{
	FixedPoint front = frontRange;
	FixedPoint up = upRange, down = downRange;
	
	//TODO: Also check if enemy is in throwable state.
	if(upRange - downRange > 0) //Air throw
	{ 
		FixedPoint yDif = enemy.root.y - root.y;
		int xDif = (enemy.root.x.value - root.x.value)*side;
		return (
			enemy.framePointer->frameProp.state == state::air &&
			(xDif < front.value && xDif > 0) &&
			(yDif < up && yDif > downRange)
		);
	}
	else //Ground throw
	{
		int xDif = (enemy.root.x.value - root.x.value)*side;
		return (
			enemy.framePointer->frameProp.state != state::air &&
			(xDif < front.value && xDif > 0)
		);
	}
}

int Actor::SetVectorFromTable(const sol::table &table, int side)
{
	auto vt = HitDef::getVectorTableFromTable(table);
	vel.x.value = vt.xSpeed*speedMultiplier*side;
	vel.y.value = vt.ySpeed*speedMultiplier;
	accel.x.value = vt.xAccel*speedMultiplier*side;
	accel.y.value = vt.yAccel*speedMultiplier;
	return lua.get()["_seqTable"][vt.sequenceName].get_or(-1);
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
		"SetVectors", &HitDef::SetVectors,
		"shakeTime", &HitDef::shakeTime
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

		"SetVector", &Actor::SetVectorFromTable,
		"ThrowCheck", &Actor::ThrowCheck,
		"Attach", [](Actor &actor, Actor &toAttach){actor.attachPoint = &toAttach;},
		"Detach", [](Actor &actor){actor.attachPoint = nullptr;},
		"RotateZ", [](Actor &actor, float amount){actor.customTransform = glm::rotate<float>(glm::mat4(1), glm::radians(amount), glm::vec3(0,0,1));},
		"RotateZP", [](Actor &actor, float amount, float x, float y){
			actor.customTransform = glm::translate(glm::rotate<float>(glm::translate(glm::mat4(1), glm::vec3(x,y,0)), glm::radians(amount), glm::vec3(0,0,1)), glm::vec3(-x,-y,0));},
		"ResetTransform", [](Actor &actor){actor.customTransform = glm::mat4(1);},

		"SpawnChild", &Actor::SpawnChild,

		"currentFrame", sol::readonly(&Actor::currFrame),
		"currentSequence", sol::readonly(&Actor::currSeq),
		"subframeCount", sol::readonly(&Actor::subframeCount),
		"totalSubframeCount", sol::readonly(&Actor::totalSubframeCount),

		"hitDef", &Actor::attack,
		"hittable", &Actor::hittable,
		"comboType", sol::readonly(&Actor::comboType),
		"userData", &Actor::userData,
		"flags", &Actor::flags,
		"frozen", &Actor::frozen,
		"landingFrame", &Actor::landingFrame,
		"hitStop", &Actor::hitstop
	);

	auto table = lua["_hit"].get_or_create<sol::table>();
	table["bounce"] = HitDef::canBounce;
	table["hitsStand"] = HitDef::hitsStand;
	table["hitsCrouch"] = HitDef::hitsCrouch;
	table["hitsAir"] = HitDef::hitsAir;
	table["unblockable"] = HitDef::unblockable;
}

void HitDef::SetVectors(int state, sol::table onHitTbl, sol::table onBlockTbl)
{
	sol::table *luaTables[2] = {&onHitTbl, &onBlockTbl};
	for(int i = 0; i < 2; i++)
	{
		Vector &vt = vectorTables[state][i];
		vt = getVectorTableFromTable(*luaTables[i]);
	}
}

HitDef::Vector HitDef::getVectorTableFromTable(const sol::table &t)
{
	Vector vt;
	vt.maxPushBackTime = t["maxTime"].get_or(0x7FFFFFFF);
	vt.xSpeed = t["xSpeed"].get_or(0);
	vt.ySpeed = t["ySpeed"].get_or(0);
	vt.xAccel = t["xAccel"].get_or(0);
	vt.yAccel = t["yAccel"].get_or(0);
	vt.sequenceName = t["ani"].get_or(std::string());
	vt.bounceTable = t["onBounce"].get_or(std::string());
	return vt;
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
	shakeTime = 0;
	vectorTables.clear();
}

