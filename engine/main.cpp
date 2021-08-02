#include "main.h"

#include <glad/glad.h>
#include <SDL.h>
#include <glm/ext/matrix_transform.hpp>

#include <deque>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <unordered_map>
#include <vector>

//#include "audio.h"
#include "camera.h"
#include "chara.h"
#include "hud.h"
#include "raw_input.h"
#include <shader.h>
#include <texture.h>
#include "util.h"
#include "window.h"
#include <vao.h>
#include <gfx_handler.h>

int inputDelay = 0;

//TODO: Remove
const char *texNames[] ={
	"data/images/background.png",
	"data/images/hud.png",
	"data/images/font.png"
	};

int gameState = GS_MENU;


int main(int argc, char** argv)
{
	mainWindow = new Window();

	//reads configurable keys
	std::ifstream keyfile("keyconf.bin", std::ifstream::in | std::ifstream::binary);
	if(keyfile.is_open())
	{
		keyfile.read((char*)modifiableSCKeys, sizeof(int)*buttonsN*2);
		keyfile.close();
	}
	keyfile.open("joyconf.bin", std::ifstream::in | std::ifstream::binary);
	if(keyfile.is_open())
	{
		keyfile.read((char*)modifiableJoyKeys, sizeof(int)*4);
		keyfile.close();
	}

	while(!mainWindow->wantsToClose)
	{
		switch (gameState)
		{
		case GS_MENU:
			//break; Not implemented so have a fallthru.
		case GS_CHARSELECT:
			//break;
		case GS_PLAY:
			PlayLoop();
			break;
		}
	}

	return 0;
}

int LoadPaletteTEMP()
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

void PlayLoop()
{
	//TODO: Remove
	std::vector<Texture> activeTextures;
	activeTextures.reserve(3);
	for(int i = 0; i < 3; ++i)
	{
		Texture texture;

		texture.Load(texNames[i]);
		
		if(i<2)
			texture.Apply(false,true);
		else
			texture.Apply();

		texture.Unload();
		activeTextures.push_back(std::move(texture));
	}
	LoadPaletteTEMP();
	
	//hud load
	std::vector<Bar> barHandler = InitBars();

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

	Camera view(1.5);

	Character player(-50, 1, "data/char/vaki/vaki.char");
	Character player2(50, -1, "data/char/vaki/vaki.char");
	
	GfxHandler gfx;
	mainWindow->context.PushShaderUboBind(&gfx.indexedS);
	mainWindow->context.PushShaderUboBind(&gfx.rectS);
	gfx.LoadGfxFromDef("data/char/vaki/def.lua");
	gfx.LoadingDone();

	player.SetCameraRef(&view);
	player2.SetCameraRef(&view);
	//player.SpawnChild();

	player.setTarget(&player2);
	player2.setTarget(&player);

	input_deque keyBufDelayed[2] = {input_deque(32, 0), input_deque(32, 0)};
	input_deque keyBuf[2]= {input_deque(32, 0), input_deque(32, 0)};


	VaoTexOnly.Bind();
	glClearColor(1, 1, 1, 1.f); 

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

		std::list<Actor*> updateList;
		player.GetAllChildren(updateList);
		player2.GetAllChildren(updateList);
		for(auto actor: updateList)
			actor->Update();

		

		Character::Collision(&player, &player2);

		barHandler[B_P1Life].Resize(player.getHealthRatio(), 1);
		barHandler[B_P2Life].Resize(player2.getHealthRatio(), 1);
		VaoTexOnly.UpdateBuffer(hudId, GetHudData().data(), GetHudData().size()*sizeof(float));
		

		//Start rendering
		glClear(GL_COLOR_BUFFER_BIT);
		
		//Should calculations be performed earlier? Watchout for this (Why?)
		glm::mat4 viewMatrix = view.Calculate(player.getXYCoords(), player2.getXYCoords());
		mainWindow->context.SetModelView(viewMatrix);

		//Draw stage quad
		glBindTexture(GL_TEXTURE_2D, activeTextures[T_STAGELAYER1].id);
		VaoTexOnly.Draw(stageId, 0, GL_TRIANGLE_FAN);

		//Draw characters
  		gfx.Begin();
		gfx.SetPaletteSlot(1);

		std::list<Actor*> drawList;
		player2.GetAllChildren(drawList);
		for(auto actor : drawList)
		{
			mainWindow->context.SetModelView(viewMatrix*actor->GetSpriteTransform());
			gfx.Draw(actor->GetSpriteIndex());
		}
		drawList.clear();

		gfx.SetPaletteSlot(0);
		player.GetAllChildren(drawList);
		for(auto actor : drawList)
		{
			mainWindow->context.SetModelView(viewMatrix*actor->GetSpriteTransform());
			gfx.Draw(actor->GetSpriteIndex());
		}

/* 		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		gfx.Draw(2000);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); */
		gfx.End();

		//Draw HUD
		VaoTexOnly.Bind();
		mainWindow->context.SetShader(RenderContext::DEFAULT);
		mainWindow->context.SetModelView();
		glBindTexture(GL_TEXTURE_2D, activeTextures[T_HUD].id);
		VaoTexOnly.Draw(hudId); 

		timerString.seekp(0);
		timerString << "SFP: " << mainWindow->GetSpf() << " FPS: " << 1/mainWindow->GetSpf();

		glBindTexture(GL_TEXTURE_2D, activeTextures[T_FONT].id);
		int count = DrawText(timerString.str(), textVertData, 2, 10);
		VaoTexOnly.UpdateBuffer(textId, textVertData.data());
		VaoTexOnly.Draw(textId, count);

		//End drawing.
		++gameTicks;
		mainWindow->SwapBuffers();
		mainWindow->SleepUntilNextFrame();
	}
}

void EventLoop()
{
	SDL_Event event;
	while(SDL_PollEvent(&event))
	{
		switch(event.type)
		{
			case SDL_QUIT:
				mainWindow->wantsToClose = true;
				return;
			case SDL_WINDOWEVENT:
				switch(event.window.event)
				{
					case SDL_WINDOWEVENT_SIZE_CHANGED:
						mainWindow->context.UpdateViewport(event.window.data1, event.window.data2);
						break;
				}
				break;
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				GameLoopKeyHandle(event.key);
				break;
		}
	}
}
