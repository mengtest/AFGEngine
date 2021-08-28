#ifndef BATTLE_SCENE_H_GUARD
#define BATTLE_SCENE_H_GUARD
#include "camera.h"
#include "texture.h"
#include "hud.h"
#include <particle.h>
#include <shader.h>
#include <ubo.h>
#include <glm/mat4x4.hpp>


class BattleScene
{
public:
	ParticleGroup pg;
	Camera view{1.55};
	int timer;

	BattleScene();
	~BattleScene();

	void PlayLoop();

private:
	std::vector<Texture> activeTextures;

	//Renderer stuff
	glm::mat4 projection;
	Ubo uniforms;
	Shader defaultS;
	unsigned int paletteId;

	void SetModelView(glm::mat4 &view);
	void SetModelView(glm::mat4 &&view);
};

#endif /* BATTLE_SCENE_H_GUARD */