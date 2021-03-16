#include "window.h"

#include <glad/glad.h>
#include <SDL.h>

#include <assert.h>
#include <iostream>
#include <chrono>
#include <stdexcept>
#include <thread>

#include "main.h"
#include "shader.h"

Window *mainWindow = nullptr;

Window::Window() :
wantsToClose(false),
fullscreen(false),
vsync(true),
busyWait(false),
window(nullptr),
frameRateChoice(0),
targetSpf(0.01666),
realSpf(0)
{
	if(SDL_Init(SDL_INIT_VIDEO))
	{
		std::cerr << SDL_GetError();
		throw std::runtime_error("Couldn't init SDL.");
	}

	/*
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	*/

	assert(!fullscreen);
	window = SDL_CreateWindow(
		"Another Fighting Game Engine",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, internalWidth*2, internalHeight*2,
		SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);

	if(!window)
	{
		std::cerr << SDL_GetError();
		SDL_Quit();
		throw std::runtime_error("Couldn't create window.");
	}

	glcontext = SDL_GL_CreateContext(window);
	if (!gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress))
	{
		SDL_DestroyWindow( window );
		SDL_Quit();
		throw std::runtime_error("Glad couldn't initialize OpenGL context.");
	}

	std::cout << "OpenGL " << GLVersion.major <<"."<< GLVersion.minor <<"\n";

	if(vsync)
		SDL_GL_SetSwapInterval(1);
}

Window::~Window()
{
	SDL_GL_DeleteContext(glcontext); 
	SDL_DestroyWindow( window );
	SDL_Quit();
}

void Window::SleepUntilNextFrame()
{
	/* TODO
	if(!vsync)
	{
		if(busyWait) //This is a workaround to some faulty sleep behaviour.
		{
			while( (realSpf = glfwGetTime()) <= targetSpf)
			{
				
			}
		}
		else
		{
			double time = glfwGetTime();
			if(time < targetSpf)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
			while( (realSpf = glfwGetTime()) <= targetSpf);
		}
	}
	glfwSetTime(0);
	*/
}

void Window::SwapBuffers()
{
	SDL_GL_SwapWindow(window);
}


void Window::GlSetup2d()
{

	glEnable(GL_MULTISAMPLE  );

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, internalWidth, 0, internalHeight, 1, -1); //The difference should equal the borders of the viewport for pixel perfection.

	int width, height;
	SDL_GetWindowSize(window, &width, &height);
	glViewport(0, 0, width, height);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_DEPTH_TEST);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glTexEnvf(GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_MODULATE); //Vertex color, maybe.

	/*glEnable(GL_POINT_SPRITE);
	glPointSize(128);
	glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);*/

	globalShaderProgram = LoadShader("data/vertex.txt", "data/fragment.txt");
	glUseProgram(globalShaderProgram);
	glUniform1i(glGetUniformLocation(globalShaderProgram, "tex0"), 0 ); //Set texture unit to be accessed as 0.
}

double Window::GetSpf()
{
	return realSpf;
}

void Window::ChangeFramerate()
{
	constexpr double framerateList[4] = {
		0.01666, 
		1.0/30.0, 
		1.0/10.0,
		1.0/2.0
	};

	frameRateChoice++;
	if(frameRateChoice >= 4)
		frameRateChoice = 0;
	targetSpf = framerateList[frameRateChoice];
}