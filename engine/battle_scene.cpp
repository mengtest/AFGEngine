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
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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

	sol::state state1, state2;
	Character player(-50, 1, "data/char/vaki/vaki.char", *this, state1);
	Character player2(50, -1, "data/char/vaki/vaki.char", *this, state2);
	
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
	
	player.setTarget(&player2);
	player2.setTarget(&player);

	input_deque keyBufDelayed[2] = {input_deque(32, 0), input_deque(32, 0)};
	input_deque keyBuf[2]= {input_deque(32, 0), input_deque(32, 0)};


	VaoTexOnly.Bind();
	glClearColor(1, 1, 1, 1.f); 
	std::vector<Actor*> drawList;
	std::vector<Actor*> updateList;

	std::vector<Particle> particles;

	defaultS.Use();
	int32_t gameTicks = 0;
	bool gameOver = false;
	while(!gameOver && !mainWindow->wantsToClose)
	{
		if(int err = glGetError())
		{
			std::cerr << "GL Error: 0x" << std::hex << err << "\n";
		}
		
		EventLoop();
		
		// TODO
		//if(glfwJoystickPresent(GLFW_JOYSTICK_1))
		//	GameLoopJoy();
		
		for(int i = 0; i < 2; ++i)
		{
			keyBuf[i].pop_back();
			keyBuf[i].push_front(keySend[i]);
			keyBufDelayed[i].pop_back();
			keyBufDelayed[i].push_front(keyBuf[i][inputDelay]);
		}

		Character::HitCollision(player, player2, keyBufDelayed[0].front(), keyBufDelayed[1].front());
		player.Input(keyBufDelayed[0]);
		player2.Input(keyBufDelayed[1]);

		updateList.clear();
		player.GetAllChildren(updateList);
		player2.GetAllChildren(updateList);

		for(auto actor: updateList)
		{
			actor->Update();
			/* if(actor->Update())
				actor->SendHitboxData(hr); */
		}
		
		Character::Collision(&player, &player2);

		auto &&pos = player.getXYCoords();
		//pg.PushNormalHit(5, 256, 128);
		pg.Update();
		pg.FillParticleVector(particles);

		barHandler[B_P1Life].Resize(player.getHealthRatio(), 1);
		barHandler[B_P2Life].Resize(player2.getHealthRatio(), 1);
		VaoTexOnly.UpdateBuffer(hudId, GetHudData().data(), GetHudData().size()*sizeof(float));
		
		//Start rendering
		glClear(GL_COLOR_BUFFER_BIT);
		
		//Should calculations be performed earlier? Watchout for this (Why?)
		glm::mat4 viewMatrix = view.Calculate(player.getXYCoords(), player2.getXYCoords());
		SetModelView(viewMatrix);

		//Draw stage quad
		glBindTexture(GL_TEXTURE_2D, activeTextures[T_STAGELAYER1].id);
		VaoTexOnly.Draw(stageId, GL_TRIANGLE_FAN);

		//Draw characters
  		gfx.Begin();
		
 		gfx.SetPaletteSlot(1);
		drawList.clear();
		player2.GetAllChildren(drawList);

		for(auto actor : drawList)
		{
			SetModelView(viewMatrix*actor->GetSpriteTransform());
			gfx.Draw(actor->GetSpriteIndex());
		}
		
		gfx.SetPaletteSlot(0);
		drawList.clear();
		player.GetAllChildren(drawList);
		for(auto actor : drawList)
		{
			SetModelView(viewMatrix*actor->GetSpriteTransform());
			gfx.Draw(actor->GetSpriteIndex());
		} 
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		SetModelView(viewMatrix);
		gfx.DrawParticles(particles, 2000);
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
		timerString << "SFP: " << mainWindow->GetSpf() << " FPS: " << 1/mainWindow->GetSpf()<<"      Entities:"<<updateList.size()<<"  "
			<< "Particles:"<<particles.size()<<"  ";

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