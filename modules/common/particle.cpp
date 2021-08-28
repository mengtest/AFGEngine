#include "particle.h"
#include "xorshift.h"
#include <cmath>

constexpr float pi = 3.1415926535897931;

ParticleGroup::ParticleGroup(XorShift32 &rng):
rng(rng)
{}

ParticleGroup& ParticleGroup::operator=(const ParticleGroup &p)
{
	particles = p.particles;
	return *this;
}

void ParticleGroup::Update()
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
		float angle = 2*pi*(float)(rng.GetU())/max32u;
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
		p.vel[0] = maxSpeed*(float)(rng.Get())/max32;
		p.vel[1] = maxSpeed*(float)(rng.Get())/max32;	
		p.acc[0] = p.vel[0]*deceleration;
		p.acc[1] = p.vel[1]*deceleration - 0.1;
		p.growRate[0] = 0.97f - 0.05*(float)(rng.GetU())/max32u;
		p.growRate[1] = p.growRate[0];
		p.remainingTicks = 20;
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
		p.vel[0] = maxSpeed*(float)(rng.Get())/max32;
		p.vel[1] = maxSpeed*(float)(rng.Get())/max32;

		float sqvx = p.vel[0]*p.vel[0];
		float sqvy = p.vel[1]*p.vel[1];
		p.p.cos = p.vel[1] /20;
		p.p.sin = p.vel[0] /20;
		p.acc[0] = p.vel[0]*deceleration;
		p.acc[1] = p.vel[1]*deceleration + 0.05;
		p.growRate[0] = 0.95f - 0.05*(float)(rng.GetU())/max32u;
		p.growRate[1] = p.growRate[0];
		p.remainingTicks = 20;
	}
}