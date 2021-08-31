#include "particle.h"
#include "xorshift.h"
#include <cmath>

constexpr float pi = 3.1415926535897931;

ParticleGroup::ParticleGroup(int type)
{
	switch (type)
	{
		default:
		case redSpark:
			updatefunc = &ParticleGroup::UpdateNormal;
			break;
		case stars:
			updatefunc = &ParticleGroup::UpdateStars;
			break;
	}
}

ParticleGroup::ParticleGroup(XorShift32 &_rng, int type): ParticleGroup(type)
{
	rng = &_rng;
}

ParticleGroup& ParticleGroup::operator=(const ParticleGroup &p)
{
	particles = p.particles;
	return *this;
}

void ParticleGroup::Update()
{
	updatefunc(this);
}

void ParticleGroup::UpdateNormal()
{

	for(int pi = 0, end = particles.size(); pi < end; ++pi)
	{
		auto& particle = particles[pi];
		while(particle.remainingTicks < 0 && pi < end)
		{
			particle = particles.back();
			particles.pop_back();
			--end;
		}
		for(int i = 0; i < 2; ++i)
		{
			particle.p.pos[i] += particle.vel[i];
			particle.vel[i] += particle.acc[i];
			particle.p.scale[0] *= particle.growRate[0];
			particle.p.scale[1] *= particle.growRate[1]; 
		}
		particle.remainingTicks -= 1;
	}
}

void ParticleGroup::UpdateStars()
{

	for(int pi = 0, end = particles.size(); pi < end; ++pi)
	{
		auto& particle = particles[pi];
		while((particle.remainingTicks < 0 || particle.p.scale[0] < 0.f) && pi < end)
		{
			particle = particles.back();
			particles.pop_back();
			--end;
		}
		if(particle.p.pos[1] < 32)
			particle.vel[1] = -particle.vel[1];
		for(int i = 0; i < 2; ++i)
		{
			particle.p.pos[i] += particle.vel[i];
			particle.vel[i] += particle.acc[i];
			particle.p.scale[0] -= particle.growRate[0];
			particle.p.scale[1] -= particle.growRate[1];
			float x = particle.p.sin;
			float y = particle.p.cos;
			particle.p.sin = x*particle.rotCos - y*particle.rotSin;
			particle.p.cos = x*particle.rotSin + y*particle.rotCos;
		}
		particle.remainingTicks -= 1;
	}
}

void ParticleGroup::FillParticleVector(std::vector<Particle> &v)
{
	int size = particles.size();
	v.resize(size);
	for(int i = 0; i < size; ++i)
		v[size-i-1] = particles[i].p;
	//A backward order allows us to still render the lastest particles if the limit is reaching.
}

constexpr float max32 = static_cast<float>(std::numeric_limits<int32_t>::max());
constexpr float max32u = static_cast<float>(std::numeric_limits<uint32_t>::max());

void ParticleGroup::PushNormalHit(int amount, float x, float y)
{
	constexpr float deceleration = -0.02;
	constexpr float maxSpeed = 10;
	//float scale = fmax(amount/20.f, 0.5);
	int start = particles.size();
	particles.resize(start+amount);

	{
		auto &p = particles[start];
		p = {};
		float angle = 2*pi*(float)(rng->GetU())/max32u;
		p.p.sin = sin(angle);
		p.p.cos = cos(angle);
		p.p.pos[0] = x;
		p.p.pos[1] = y;
		p.p.scale[0] = 2;
		p.p.scale[1] = 2;
		p.growRate[0] = 1.05;
		p.growRate[1] = 0.85;
		p.remainingTicks = 12;
	}
	for(int i = start+1; i < start+amount; ++i)
	{
		auto &p = particles[i];
		
		p.p.cos = 1;
		p.p.sin = 0;
		p.p.pos[0] = x;
		p.p.pos[1] = y;
		p.p.scale[0] = 0.25;
		p.p.scale[1] = 0.25;
		p.vel[0] = maxSpeed*(float)(rng->Get())/max32;
		p.vel[1] = maxSpeed*(float)(rng->Get())/max32;	
		p.acc[0] = p.vel[0]*deceleration;
		p.acc[1] = p.vel[1]*deceleration - 0.1;
		p.growRate[0] = 0.97f - 0.05*(float)(rng->GetU())/max32u;
		p.growRate[1] = p.growRate[0];
		p.remainingTicks = 20;
	}
}

void ParticleGroup::PushCounterHit(int amount, float x, float y)
{
	constexpr float deceleration = 0.05;
	constexpr float maxSpeed = 10;
	//float scale = fmax(amount/20.f, 0.5);
	int start = particles.size();
	particles.resize(start+amount);
	{
		float angle = 2.f*pi*(float)(rng->Get())/max32;
		auto &p = particles[start];
		p = {};
		p.p.sin = sin(angle);
		p.p.cos = cos(angle);
		p.p.pos[0] = x;
		p.p.pos[1] = y;
		p.p.scale[0] = 1.5;
		p.p.scale[1] = 2;
		p.growRate[0] = 0.08;
		p.growRate[1] = -0.08;
		p.rotSin = 0;
		p.rotCos = 1;
		p.remainingTicks = 20;
	}
	for(int i = start+1; i < start+amount; ++i)
	{
		float angle = 0.3f*(float)(rng->Get())/max32;
		auto &p = particles[i];
		p.p.cos = 1;
		p.p.sin = 0;
		p.p.pos[0] = x;
		p.p.pos[1] = y;
		p.p.scale[0] = 0.8f - 0.5*(float)(rng->GetU())/max32u;
		p.p.scale[1] = p.p.scale[0];
		p.rotSin = sin(angle);
		p.rotCos = cos(angle);
		p.vel[0] = maxSpeed*(float)(rng->Get())/max32;
		p.vel[1] = 1*maxSpeed*(float)(rng->GetU())/max32u+2.f;
		p.acc[0] = p.vel[0]*deceleration;
		p.acc[1] = p.vel[1]*deceleration - 0.5;
		p.growRate[0] = 0.01f - 0.005*(float)(rng->GetU())/max32u;
		p.growRate[1] = p.growRate[0];
		p.remainingTicks = 60;
	}
}

void ParticleGroup::PushSlash(int amount, float x, float y)
{
	constexpr float deceleration = -0.05;
	constexpr float maxSpeed = 18;
	float scale = fmax(amount/20.f, 0.5);
	int start = particles.size();
	particles.resize(start+amount);

	{
		auto &p = particles[start];
		p = {};
		p.p.sin = 0;
		p.p.cos = 1;
		p.p.pos[0] = x;
		p.p.pos[1] = y;
		p.p.scale[0] = 2;
		p.p.scale[1] = 2;
		p.growRate[0] = 0.99;
		p.growRate[1] = 0.9;
		p.remainingTicks = 10;
	}
	for(int i = start+1; i < start+amount; ++i)
	{
		auto &p = particles[i];
		
		p.p.pos[0] = x;
		p.p.pos[1] = y;
		p.p.scale[0] = scale*8;
		p.p.scale[1] = scale*0.25;
		p.vel[0] = maxSpeed*(float)(rng->Get())/max32;
		p.vel[1] = maxSpeed*(float)(rng->Get())/max32;

		float sqvx = p.vel[0]*p.vel[0];
		float sqvy = p.vel[1]*p.vel[1];
		p.p.cos = p.vel[1] /20;
		p.p.sin = p.vel[0] /20;
		p.acc[0] = p.vel[0]*deceleration;
		p.acc[1] = p.vel[1]*deceleration + 0.05;
		p.growRate[0] = 0.9999f - 0.00001*(float)(rng->GetU())/max32u;
		p.growRate[1] = p.growRate[0];
		p.remainingTicks = 20;
	}
}