#ifndef BATTLE_INTERFACE_H_GUARD
#define BATTLE_INTERFACE_H_GUARD

#include "xorshift.h"
#include "particle.h"
#include "camera.h"

struct BattleInterface
{
	XorShift32 &rng;
	ParticleGroup &pg;
	Camera &view;
};

#endif /* BATTLE_INTERFACE_H_GUARD */
