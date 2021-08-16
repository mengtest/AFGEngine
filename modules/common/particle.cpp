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
			float g2 = particle.growRate*particle.growRate;
			particle.p.scale[0] *= g2;
			particle.p.scale[1] *= g2; 
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
	constexpr float deceleration = -0.05;
	constexpr float maxSpeed = 8;
	float scale = fmax(amount/20.f, 0.5);
	int start = particles.size();
	particles.resize(start+amount);

	{
		auto &p = particles[start];
		p = {};
		p.p.pos[0] = x;
		p.p.pos[1] = y;
		p.p.scale[0] = 2;
		p.p.scale[1] = 2;
		p.growRate = 0.90;
		p.remainingTicks = 10;
	}
	for(int i = start+1; i < start+amount; ++i)
	{
		auto &p = particles[i];
		p.p.pos[0] = x;
		p.p.pos[1] = y;
		p.p.scale[0] = scale;
		p.p.scale[1] = scale;
		p.vel[0] = maxSpeed*(float)(xorShift32.Get())/max32;
		p.vel[1] = maxSpeed*(float)(xorShift32.Get())/max32;
		p.acc[0] = p.vel[0]*deceleration;
		p.acc[1] = p.vel[1]*deceleration + 0.05;
		p.growRate = 0.95f - 0.05*(float)(xorShift32.GetU())/max32u;
		p.remainingTicks = 20;
	}
}