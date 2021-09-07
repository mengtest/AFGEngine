#include "battle_scene.h"
#include "chara.h"
#include "util.h"
#include "raw_input.h"
#include "window.h"
#include "hitbox_renderer.h"
#include "game_state.h"
#include "stage.h"
#include <gfx_handler.h>
#include <vao.h>
#include <fstream>
#include <sstream>
#include <vector>

#include <glad/glad.h> //Palette loading only. Will remove.
#include <glm/gtc/type_ptr.hpp> //Uniform matrix load.

enum //TODO: Remove
{
	T_FONT
};

//TODO: Load from lua
const char *texNames[] ={
	"data/hud/font.lzs3"
};

int inputDelay = 0;

//TODO: Remove.
unsigned int LoadPaletteTEMP()
{
	std::ifstream pltefile("data/palettes/col1.act", std::ifstream::in | std::ifstream::binary);
	uint8_t palette[256*3*2];

	pltefile.read((char*)palette, 256*3);
	pltefile.close();
	pltefile.open("data/palettes/col2.act", std::ifstream::in | std::ifstream::binary);
	pltefile.read((char*)(palette+256*3), 256*3);

	GLuint paletteGlId;

	glGenTextures(1, &paletteGlId);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, paletteGlId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 2, 0, GL_RGB, GL_UNSIGNED_BYTE, palette);
	glActiveTexture(GL_TEXTURE0);

	return paletteGlId;
}

BattleScene::BattleScene():
interface{rng, particleGroups, view},
player(interface), player2(interface),
uniforms("Common", 1)
{
	{ //Loads font texture. TODO: Remove
		Texture texture;
		texture_options opt;
		opt.linearFilter = true;
		texture.LoadLzs3(texNames[0], opt);
		activeTextures.push_back(std::move(texture));
	}
	
	paletteId = LoadPaletteTEMP();

	defaultS.LoadShader("data/def.vert", "data/def.frag");
	defaultS.Use();	

	//Bind transform matrix uniforms.
	uniforms.Init(sizeof(float)*16);
	uniforms.Bind(defaultS.program);

	projection = glm::ortho<float>(0, internalWidth, 0, internalHeight, -32768, 32767);
	//projection = glm::ortho<float>(-internalWidth*0.5, internalWidth*1.5, -internalHeight*0.5, internalHeight*1.5, -32768, 32767);
	/* projection = glm::perspective<float>(90, (float)internalWidth/(float)internalHeight, 1, 32767);
	projection = glm::rotate(projection, 0.3f, glm::vec3(0.f,1.f,0.f));
	projection = glm::translate(projection, glm::vec3(-internalWidth/2.f,-internalHeight/2.f,-200)); */
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glClearColor(1, 1, 1, 1.f); 
	glClearDepth(1);


}

BattleScene::~BattleScene()
{
	glDeleteTextures(1, &paletteId);
}

int BattleScene::PlayLoop(bool replay)
{
	std::vector<uint32_t> inputs;
	size_t inputSize;
	size_t inputI = 0;
	if(replay)
	{
		std::ifstream replayFile("replay", std::ios_base::binary);
		replayFile.read((char*)&inputSize, sizeof(size_t));
		inputs.resize(inputSize);
		replayFile.read((char*)inputs.data(), sizeof(uint32_t)*inputSize);
	}
	else
		inputs.reserve(0x2000);
	
	std::ostringstream timerString;
	timerString.precision(6);
	timerString.setf(std::ios::fixed, std::ios::floatfield);

	Hud hud;

	int stageId, textId;
	std::vector<float> textVertData;
	textVertData.resize(24*80);
	Vao vaoTexOnly(Vao::F2F2, GL_DYNAMIC_DRAW);
	{
		hud.Load("data/hud/hud.lua", vaoTexOnly);
		textId = vaoTexOnly.Prepare(sizeof(float)*textVertData.size(), nullptr);
		vaoTexOnly.Load();
	}

	player.Load(1, "data/char/vaki/vaki.char", 0);
	player2.Load(-1, "data/char/vaki/vaki.char", 1);
	
	//For rendering purposes only.
	std::vector<float> hitboxData;
	HitboxRenderer hr;

	GfxHandler gfx;
	int playerGfx = gfx.LoadGfxFromDef("data/char/vaki/def.lua");
	//std::function<void(glm::mat4&)> setView = std::bind(static_cast<void(BattleScene::*)(glm::mat4&)>(&BattleScene::SetModelView), this, std::placeholders::_1);
	Stage stage(gfx, "data/bg/bg.lua", [&](glm::mat4& a){SetModelView(a);});
	gfx.LoadingDone();

	uniforms.Bind(hr.sSimple.program);
	uniforms.Bind(gfx.indexedS.program);
	uniforms.Bind(gfx.rectS.program);
	uniforms.Bind(gfx.particleS.program);
	
	player.SetTarget(player2);
	player2.SetTarget(player);
	player.priority = 1;

	Player* players[2];

	vaoTexOnly.Bind();

	std::vector<Particle> particles;
	Player::DrawList drawList;
	for(int i = ParticleGroup::START; i < ParticleGroup::END; ++i)
		particleGroups.insert({i, {rng, i}});

	defaultS.Use();

	auto keyHandler = std::bind(&BattleScene::KeyHandle, this, std::placeholders::_1);
	int32_t gameTicks = 0;
	bool gameOver = false;
	
	while(!gameOver && !mainWindow->wantsToClose)
	{
		if(int err = glGetError())
		{
			std::cerr << "GL Error: 0x" << std::hex << err << "\n";
		}
		
		EventLoop(keyHandler);

		if(replay)
		{
			if(inputI >= inputSize)
				break;
			player.SendInput(inputs[inputI++]);
			player2.SendInput(inputs[inputI++]);
		}
		else
		{
			inputs.push_back(keySend[0]);
			inputs.push_back(keySend[1]);
			
			player.SendInput(keySend[0]);
			player2.SendInput(keySend[1]);
		}
		
		if(player.priority >= player2.priority){
			players[0] = &player;
			players[1] = &player2;
		}else{
			players[0] = &player2;
			players[1] = &player;
		}

		Player::HitCollision(player, player2);
		players[0]->ProcessInput();
		players[1]->ProcessInput();

		players[0]->Update(hr);
		players[1]->Update(hr);
		
		Player::Collision(player, player2);
		drawList.Init(player, player2);

		//auto &&pos = players[1]->getXYCoords();
		//pg.PushNormalHit(5, 256, 128);

		//Hud state update
		//barHandler[B_P1Life].Resize(player.GetHealthRatio(), 1);
		//barHandler[B_P2Life].Resize(player2.GetHealthRatio(), 1);
		//VaoTexOnly.UpdateBuffer(hudId, GetHudData().data(), GetHudData().size()*sizeof(float));
		
		//Start rendering
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		//Should calculations be performed earlier? Watchout for this (Why?)
		glm::mat4 viewMatrix = view.Calculate(players[0]->GetXYCoords(), players[1]->GetXYCoords());

		gfx.Begin();
		//Draw stage quad
		auto center = view.GetCameraCenterScale();
		stage.Draw(viewMatrix, center);

		int p1Pos = players[1]->FillDrawList(drawList);
		int p2Pos = players[0]->FillDrawList(drawList);

		auto draw = [this,&gfx](Actor *actor, glm::mat4 &viewMatrix)
		{
			SetModelView(viewMatrix*actor->GetSpriteTransform());
			auto options = actor->GetRenderOptions();
			switch(options.blendingMode) //Maybe should go inside draw.
			{
				case RenderOptions::normal:
					glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
					break;
				case RenderOptions::additive:
					glBlendFunc(GL_SRC_ALPHA, GL_ONE);
					break;
			};
			gfx.Draw(actor->GetSpriteIndex(), 0, options.paletteIndex);
		};

		auto invertedView = glm::translate(glm::scale(viewMatrix, glm::vec3(1,-1,1)), glm::vec3(0,-64,0));

		//Draw player reflection?
		gfx.SetMulColor(1, 1, 1, 0.2);
		draw(drawList.v[p1Pos], invertedView);
		draw(drawList.v[p2Pos], invertedView);
		gfx.SetMulColor(1, 1, 1, 1);
	
		//Draw all actors
		for(auto actor : drawList.v)
		{
			draw(actor,viewMatrix);
		}

		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		SetModelView(viewMatrix);
		for(auto &pg : particleGroups)
		{
			pg.second.Update();
			pg.second.FillParticleVector(particles);
			gfx.DrawParticles(particles, pg.first);
		}
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		gfx.End();

		//Draw boxes
		/* hr.LoadHitboxVertices();
		hr.Draw(); */
		
		//Draw HUD
		SetModelView(glm::mat4(1));
		vaoTexOnly.Bind();
		defaultS.Use();

		//Draw lifebars.
		glBindTexture(GL_TEXTURE_2D, hud.texture.id);
		hud.ResizeBarId(0, (gameTicks%120)/119.f);
		hud.ResizeBarId(1, (gameTicks%240)/239.f);
		hud.ResizeBarId(2, (gameTicks%60)/59.f);
		hud.ResizeBarId(3, (gameTicks%512)/511.f);
		hud.ResizeBarId(4, player.GetHealthRatio());
		hud.ResizeBarId(5, player2.GetHealthRatio());
		hud.Draw();

		//Draw fps bar
		/* timerString.seekp(0);
		timerString << "SFP: " << mainWindow->GetSpf() << " FPS: " << 1/mainWindow->GetSpf()<<"      Entities:"<<drawList.v.size()<<
			"   Particles:"<<particles.size()<<"  ";
		glBindTexture(GL_TEXTURE_2D, activeTextures[T_FONT].id);
		int count = DrawText(timerString.str(), textVertData, 2, 10);
		vaoTexOnly.UpdateBuffer(textId, textVertData.data());
		vaoTexOnly.Draw(textId); */

		//End drawing.
		++gameTicks;
		mainWindow->SwapBuffers();
		mainWindow->SleepUntilNextFrame();
	}

	if(!replay)
	{
		size_t inputSize = inputs.size();
		std::ofstream replayFile("replay", std::ios_base::binary);
		replayFile.write((char*)&inputSize, sizeof(size_t));
		replayFile.write((char*)inputs.data(), sizeof(uint32_t)*inputSize);
	}

	return GS_WIN;
}

void BattleScene::SetModelView(glm::mat4& view)
{
	uniforms.SetData(glm::value_ptr(projection*view));
}

void BattleScene::SetModelView(glm::mat4&& view)
{
	uniforms.SetData(glm::value_ptr(projection*view));
}

void BattleScene::SaveState()
{
	state.rng = rng;
	state.particleGroups = particleGroups;
	state.p1 = player.GetStateCopy();
	state.p2 = player2.GetStateCopy();
	state.view = view;
}

void BattleScene::LoadState()
{
	if(!state.p1.charObj) //There are no saves.
		return;
	rng = state.rng;
	particleGroups = state.particleGroups;
	player.SetState(state.p1);
	player2.SetState(state.p2);
	view = state.view;
}

bool BattleScene::KeyHandle(const SDL_KeyboardEvent &e)
{
	if(e.type != SDL_KEYDOWN)
		return false;

	switch (e.keysym.scancode){
	case SDL_SCANCODE_F1: 
		SaveState();
		break;
	case SDL_SCANCODE_F2:
		LoadState();
		break;
	default:
		return false;
	}
	return true;
}
