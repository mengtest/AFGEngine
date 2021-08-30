#include "battle_scene.h"
#include "chara.h"
#include "util.h"
#include "raw_input.h"
#include "window.h"
#include "hitbox_renderer.h"
#include <gfx_handler.h>
#include <vao.h>
#include <fstream>
#include <sstream>
#include <vector>

#include <glad/glad.h> //Palette loading only. Will remove.
#include <glm/gtc/type_ptr.hpp> //Uniform matrix load.

enum //TODO: Remove
{
	T_STAGELAYER1, //multiple layers on different textures? No, idiot.
	T_HUD,
	T_FONT,
	T_CHAR
};

//TODO: Load from lua
const char *texNames[] ={
	"data/images/background.lzs3",
	"data/images/hud.lzs3",
	"data/images/font.lzs3"
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
	texture_options opt; opt.linearFilter = true;
	activeTextures.reserve(3);
	for(int i = 0; i < 3; ++i)
	{
		Texture texture;
		texture_options opt;

		if(i<3)
			opt.linearFilter = true;

		texture.LoadLzs3(texNames[i], opt);
		activeTextures.push_back(std::move(texture));
	}
	paletteId = LoadPaletteTEMP();

	

	defaultS.LoadShader("data/def.vert", "data/def.frag");
	defaultS.Use();	

	//Bind transform matrix uniforms.
	uniforms.Init(sizeof(float)*16);
	uniforms.Bind(defaultS.program);

	projection = glm::ortho<float>(0, internalWidth, 0, internalHeight, -32768, 32767);
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

void BattleScene::PlayLoop()
{
	//TODO: Get rid of this garbage hud code. It can and will cause problems.
	std::vector<Bar> barHandler;
	barHandler = InitBars();
	
	std::ostringstream timerString;
	timerString.precision(6);
	timerString.setf(std::ios::fixed, std::ios::floatfield);

	int hudId, stageId, textId;
	std::vector<float> textVertData;
	textVertData.resize(24*80);
	Vao VaoTexOnly(Vao::F2F2, GL_DYNAMIC_DRAW);
	{
		float stageVertices[] = {
			-300, 0, 	0, 0,
			300, 0,  	1, 0,
			300, 450,  	1, 1,
			-300, 450,	0, 1
		};
		
		hudId = VaoTexOnly.Prepare(GetHudData().size()*sizeof(float), nullptr);
		stageId = VaoTexOnly.Prepare(sizeof(stageVertices), stageVertices);
		textId = VaoTexOnly.Prepare(sizeof(float)*textVertData.size(), nullptr);
		VaoTexOnly.Load();
	}

	player.Load(1, "data/char/vaki/vaki.char");
	player2.Load(-1, "data/char/vaki/vaki.char");
	
	//For rendering purposes only.
	std::vector<float> hitboxData;
	HitboxRenderer hr;

	GfxHandler gfx;
	gfx.LoadGfxFromDef("data/char/vaki/def.lua");
	gfx.LoadingDone();

	uniforms.Bind(hr.sSimple.program);
	uniforms.Bind(gfx.indexedS.program);
	uniforms.Bind(gfx.rectS.program);
	uniforms.Bind(gfx.particleS.program);
	
	player.SetTarget(player2);
	player2.SetTarget(player);

	VaoTexOnly.Bind();

	std::vector<Particle> particles;
	for(int i = ParticleGroup::START; i < ParticleGroup::END; ++i)
		particleGroups.insert({i, {rng, i}});

	defaultS.Use();

	auto f = std::bind(&BattleScene::KeyHandle, this, std::placeholders::_1);
	int32_t gameTicks = 0;
	bool gameOver = false;
	while(!gameOver && !mainWindow->wantsToClose)
	{
		if(int err = glGetError())
		{
			std::cerr << "GL Error: 0x" << std::hex << err << "\n";
		}
		
		EventLoop(f);
		
		// TODO
		//if(glfwJoystickPresent(GLFW_JOYSTICK_1))
		//	GameLoopJoy();
		
		player.SendInput(keySend[0]);
		player2.SendInput(keySend[1]);

		Player::HitCollision(player, player2);
		player.ProcessInput();
		player2.ProcessInput();

		player.Update(hr);
		player2.Update(hr);
		
		Player::Collision(player, player2);

		//auto &&pos = player.getXYCoords();
		//pg.PushNormalHit(5, 256, 128);


		barHandler[B_P1Life].Resize(player.GetHealthRatio(), 1);
		barHandler[B_P2Life].Resize(player2.GetHealthRatio(), 1);
		VaoTexOnly.UpdateBuffer(hudId, GetHudData().data(), GetHudData().size()*sizeof(float));
		
		//Start rendering
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		//Should calculations be performed earlier? Watchout for this (Why?)
		glm::mat4 viewMatrix = view.Calculate(player.GetXYCoords(), player2.GetXYCoords());
		SetModelView(viewMatrix);

		//Draw stage quad
		glBindTexture(GL_TEXTURE_2D, activeTextures[T_STAGELAYER1].id);
		VaoTexOnly.Draw(stageId, GL_TRIANGLE_FAN);

		//Draw characters
  		gfx.Begin();
		
 		gfx.SetPaletteSlot(1);

		for(auto actor : player2.updateList)
		{
			SetModelView(viewMatrix*actor->GetSpriteTransform());
			gfx.Draw(actor->GetSpriteIndex());
		}
		
		gfx.SetPaletteSlot(0);
		for(auto actor : player.updateList)
		{
			SetModelView(viewMatrix*actor->GetSpriteTransform());
			gfx.Draw(actor->GetSpriteIndex());
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
		VaoTexOnly.Bind();
		defaultS.Use();
		glBindTexture(GL_TEXTURE_2D, activeTextures[T_HUD].id);
		VaoTexOnly.Draw(hudId); 

		timerString.seekp(0);
		timerString << "SFP: " << mainWindow->GetSpf() << " FPS: " << 1/mainWindow->GetSpf()<<"      Entities:"<<player.updateList.size()<<" - "
			<<player2.updateList.size()<< "   Particles:"<<particles.size()<<"  ";

		glBindTexture(GL_TEXTURE_2D, activeTextures[T_FONT].id);
		int count = DrawText(timerString.str(), textVertData, 2, 10);
		VaoTexOnly.UpdateBuffer(textId, textVertData.data());
		VaoTexOnly.Draw(textId);
		//End drawing.
		++gameTicks;
		mainWindow->SwapBuffers();
		mainWindow->SleepUntilNextFrame();
	}
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
	rng = state.rng;
	particleGroups = state.particleGroups;
	player.SetState(state.p1);
	player2.SetState(state.p2);
	view = state.view;
}

void BattleScene::KeyHandle(SDL_KeyboardEvent &e)
{
	if(e.repeat)
		return;
	
	SDL_Scancode scancode = e.keysym.scancode; 

	bool action = e.type == SDL_KEYDOWN;
	for(int i = 0; i < buttonsN; ++i)
	{
		if(scancode == modifiableSCKeys[i])
		{
			if(action)
				keySend[0] |= 1 << i;
			else
				keySend[0] &= ~(1 << i);
		}
	}
	for(int i = 0; i < buttonsN; ++i)
	{
		if(scancode == modifiableSCKeys[i+buttonsN])
		{
			if(action)
				keySend[1] |= 1 << i;
			else
				keySend[1] &= ~(1 << i);
		}
	}


	//unmodifiable keys.
	if(!action)
		return;

	switch (scancode){
	case SDL_SCANCODE_F9: //Set custom keys for player one
		SetupKeys(0);
		break;
	case SDL_SCANCODE_F10: //and player two too
		SetupKeys(buttonsN);
		break;
	case SDL_SCANCODE_F11: //Set joy buttons
		{
			/* TODO
			int buttonN = 0;
			glfwGetJoystickButtons(GLFW_JOYSTICK_1, &buttonN);
			int i = 0;
			int *used = new int[buttonN];
			while(i < 4)
			{
				const unsigned char *buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &buttonN);
				for(int buttonItr = 0; buttonItr < buttonN; ++buttonItr)
				{
					if(buttons[buttonItr] == GLFW_PRESS)
					{
						if(used[buttonItr] == 1)
							continue;
						used[buttonItr] = 1;
						modifiableJoyKeys[i] = buttonItr;
						++i;
						break;
					}
				}
			}
			delete[] used;
			std::ofstream keyfile("joyconf.bin", std::ofstream::out | std::ofstream::binary);
			keyfile.write((const char*)modifiableJoyKeys, sizeof(int)*4);
			keyfile.close();
			*/
		}
		break;
	case SDL_SCANCODE_F5: //Switches between different framerates
		mainWindow->ChangeFramerate();
		break;
	case SDL_SCANCODE_F1: 
		SaveState();
		break;
	case SDL_SCANCODE_F2:
		LoadState();
		break;
	case SDL_SCANCODE_ESCAPE:
			mainWindow->wantsToClose = true;
		break;
	default:
		return;
	}
}