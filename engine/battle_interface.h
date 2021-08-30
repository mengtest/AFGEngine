#ifndef BATTLE_INTERFACE_H_GUARD
#define BATTLE_INTERFACE_H_GUARD

#include "xorshift.h"
#include "particle.h"
#include "camera.h"
#include <unordered_map>

struct BattleInterface
{
	XorShift32 &rng;
	std::unordered_map<int, ParticleGroup> &particles;
	Camera &view;
};

#endif /* BATTLE_INTERFACE_H_GUARD */
