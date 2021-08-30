#ifndef PARTICLE_H_GUARD
#define PARTICLE_H_GUARD

#include <functional>
#include <vector>
#include "xorshift.h"

struct Particle{
	float pos[2];
	float scale[2];
	float sin;
	float cos;
};

class ParticleGroup
{
private:
	struct ParticleState{
		Particle p;
		float vel[2];
		float acc[2];
		float growRate[2];
		float rotSin; //Rotation speed
		float rotCos;
		int remainingTicks;
	};	

	std::vector<ParticleState> particles;

	std::function<void(ParticleGroup*)> updatefunc;
	void UpdateNormal();
	void UpdateStars();

public:
	enum type{
		START = 2000,
		redSpark = START,
		stars,
		END
	};

	ParticleGroup(int type = START);
	ParticleGroup(XorShift32& rng, int type = START);
	ParticleGroup& operator=(const ParticleGroup &p);
	XorShift32 *rng = nullptr;
	void Update();
	void FillParticleVector(std::vector<Particle> &v);
	void PushNormalHit(int amount, float x, float y);
	void PushCounterHit(int amount, float x, float y);
	void PushSlash(int amount, float x, float y);

	
};

#endif /* PARTICLE_H_GUARD */
