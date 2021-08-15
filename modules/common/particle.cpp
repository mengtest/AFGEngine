#include "particle.h"
#include "xorshift.h"

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
			particle.p.scale[0] *= particle.growRate;
			particle.p.scale[1] *= particle.growRate; 
		}
		particle.remainingTicks -= 1;
	}
}

void ParticleGroup::FillParticleVector(std::vector<Particle> &v)
{
	int size = particles.size();
	v.resize(size);
	for(int i = 0; i < size; ++i)
		v[i] = particles[i].p;
}

constexpr float max32 = static_cast<float>(std::numeric_limits<int32_t>::max());
constexpr float max32u = static_cast<float>(std::numeric_limits<uint32_t>::max());

void ParticleGroup::PushNormalHit(int amount, float x, float y)
{
	constexpr float deceleration = -0.05;
	constexpr float maxSpeed = 8;
	float scale = amount/20.f;
	int start = particles.size();
	particles.resize(start+amount);
	for(int i = start; i < start+amount; ++i)
	{
		auto &p = particles[i];
		p.p.pos[0] = x;
		p.p.pos[1] = y;
		p.p.scale[0] = scale;
		p.p.scale[1] = scale;
		p.vel[0] = maxSpeed*(float)(xorShift32.Get())/max32;
		p.vel[1] = maxSpeed*(float)(xorShift32.Get())/max32;
		p.acc[0] = p.vel[0]*deceleration;
		p.acc[1] = p.vel[1]*deceleration;
		p.growRate = 0.9f - 0.1*(float)(xorShift32.GetU())/max32u;
		p.remainingTicks = 20;
	}
}