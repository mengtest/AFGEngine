#include "main.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <deque>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <vector>

#include "audio.h"
#include "camera.h"
#include "chara.h"
#include "hud.h"
#include "raw_input.h"
#include "shader.h"
#include "util.h"
#include "window.h"

#ifdef NETPLAY
#include "netplay.h"
#endif

int inputDelay = 0;

const float charTexCoords[8] = {0,0,1,0,1,1,0,1};

const char *texNames[] ={
	"images/background.png",
	"images/glow.png",
	"images/hud.png",
	"images/fontactive.png",
	"images/fontdead.png"
	};

int gameState = GS_MENU;


int main(int argc, char** argv)
{

	#ifdef NETPLAY
	if(argc > 1) //Netplay
	{
		if(!NetplayArgs(argc, argv))
			return 0;
	}
	#endif

	mainWindow = InitWindow();
	if(!mainWindow)
		return EXIT_FAILURE;

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

	while (!glfwWindowShouldClose(mainWindow))
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

	glfwDestroyWindow(mainWindow);
	glfwTerminate();
	return 0;
}

void PlayLoop()
{
	GlSetup2d();
	glUseProgram(globalShaderProgram);
	glfwSetKeyCallback(mainWindow, CbGameLoopKeys);

	

	//texture load
	std::vector<Texture> activeTextures;
	activeTextures.resize(5);
	for(int i = 0; i < 5; ++i)
	{
		activeTextures[i].Load(texNames[i]);
		if(i<3)
			activeTextures[i].Apply();
		else
			activeTextures[i].Apply(false, false);
	}

	//fontSettings testfont = {32, 8, &activeTextures[T_FONT]};

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	//hud load

	std::vector<Bar> barHandler = InitBars();


	glUseProgram(globalShaderProgram);

	int32_t gameTicks = 0;

	pal = 1;
	Character player(200, 1, "char/vaki.char");
	pal = 0;
	Character player2(250, -1, "char/vaki.char");

	Camera view(1.1);
	player.SetCameraRef(&view);
	player2.SetCameraRef(&view);

	player.setTarget(&player2);
	player2.setTarget(&player);


	input_deque keyBufDelayed[2] = {input_deque(32, key::buf::TRUE_NEUTRAL), input_deque(32, key::buf::TRUE_NEUTRAL)};
	input_deque keyBuf[2]= {input_deque(32, key::buf::TRUE_NEUTRAL), input_deque(32, key::buf::TRUE_NEUTRAL)};
	/*input_deque dummy(32, key::buf::TRUE_NEUTRAL);
	dummy[3] |= key::buf::LEFT;
	dummy[2] |= key::buf::DOWN;
	dummy[1] |= key::buf::LEFT | key::buf::DOWN;
	dummy[0] |= key::buf::UP;*/

	std::ostringstream timerString;
	timerString.precision(6);
	timerString.setf(std::ios::fixed, std::ios::floatfield);

	/*Song testA("audio/372094.ogg");
	testA.playback.startStream();*/

	bool gameOver = false;
	while(!gameOver && !glfwWindowShouldClose(mainWindow))
	{
		for(int i = 0; i < 2; ++i)
		{
			keyBuf[i].pop_back();
			keyBuf[i].push_front(keySend[i]);
			keyBufDelayed[i].pop_back();
			keyBufDelayed[i].push_front(keyBuf[i][inputDelay]);
		}

		player.HitCollision();
		player2.HitCollision();

		glfwPollEvents();
		if(glfwJoystickPresent(GLFW_JOYSTICK_1))
			GameLoopJoy();
		player.Input(&keyBufDelayed[0]);
		player2.Input(&keyBufDelayed[1]);

		player.Update();
		player2.Update();

		Character::Collision(&player, &player2);
		

		/*player.Print();
		player2.Print();
		std::cout << " \n";*/

		barHandler[B_P1Life].Resize(player.getHealthRatio(), 1);
		barHandler[B_P2Life].Resize(player2.getHealthRatio(), 1);

		//Turn this into a draw function perhaps?
		//glClear(GL_COLOR_BUFFER_BIT);
		glPushMatrix();
		view.Calculate(player.getXYCoords(), player2.getXYCoords());
		view.Apply();

		//This stays until stage files exist and can be loaded.
		glBindTexture(GL_TEXTURE_2D, activeTextures[T_STAGELAYER1].id);
		glBegin(GL_QUADS );
			glColor3f(1, 0.9, 0.8);
			glTexCoord2f(0, 0);
			glVertex3f(-480, 0, 0);

			glTexCoord2f(1, 0);
			glVertex3f(480, 0, 0);

			glColor3f(0, 0, 0);
			glTexCoord2f(1, 1);
			glVertex3f(480, 540, 0);

			glTexCoord2f(0, 1);
			glVertex3f(-480, 540, 0);
			glColor3f(1, 1, 1);
		glEnd();


		glTexCoordPointer(2, GL_FLOAT, 0, charTexCoords);


		player2.Draw();
		player.Draw();

		glPopMatrix();
		DrawHud(activeTextures[T_HUD].id);

		timerString.seekp(0);
		timerString << "SFP: " << GetSpf() << " FPS: " << 1/GetSpf();

		DrawTextA(timerString.str(), activeTextures[T_FONT].id, 2, 12, 0);
		/* DrawText("the quick brown fox jumps over the lazy dog", texturelist[2], -170, 100, -200); //Font texting
		DrawText("The Quick Brown Fox Jumps Over The Lazy dog", texturelist[2], -170, 80, -200);
		DrawText("THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG", texturelist[2], -170, 60, -200);*/

		glfwSwapBuffers(mainWindow);

		//End drawing.
		 ++gameTicks;

		SleepUntilNextFrame();
	}
}
