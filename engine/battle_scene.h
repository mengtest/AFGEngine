#ifndef BATTLE_SCENE_H_GUARD
#define BATTLE_SCENE_H_GUARD
#include "camera.h"

struct BattleScene
{
	Camera view{1.05};
	int timer;
};

#endif /* BATTLE_SCENE_H_GUARD */