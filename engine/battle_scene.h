#ifndef BATTLE_SCENE_H_GUARD
#define BATTLE_SCENE_H_GUARD

#include "battle_interface.h"
#include "chara.h"
#include "camera.h"
#include "texture.h"
#include "hud.h"
#include "xorshift.h"
#include <particle.h>
#include <shader.h>
#include <ubo.h>
#include <glm/mat4x4.hpp>
#include <SDL_events.h>

struct SaveState
{
	XorShift32 rng;
	std::unordered_map<int, ParticleGroup> particleGroups;
	Camera view;
	PlayerStateCopy p1, p2;

	//SaveState():{}
};

class BattleScene
{
private:
	XorShift32 rng;
	std::unordered_map<int, ParticleGroup> particleGroups;
	Camera view{1.55};
	int timer;

	BattleInterface interface;
	Player player, player2;

	SaveState state;

public:
	BattleScene();
	~BattleScene();
	void SaveState();
	void LoadState();

	int PlayLoop();

private:
	std::vector<Texture> activeTextures;

	//Renderer stuff
	glm::mat4 projection;
	Ubo uniforms;
	Shader defaultS;
	unsigned int paletteId;

	void SetModelView(glm::mat4 &view);
	void SetModelView(glm::mat4 &&view);
	void KeyHandle(SDL_KeyboardEvent &e);
};

#endif /* BATTLE_SCENE_H_GUARD */