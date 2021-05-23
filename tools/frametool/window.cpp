#include "window.h"
#include "ini.h"
#include <shader.h>

#include <glad/glad.h>
#include <SDL.h>

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl.h>

#include <assert.h>
#include <iostream>
#include <chrono>
#include <thread>

Window *mainWindow = nullptr;

Window::Window() :
wantsToClose(false),
fullscreen(false),
vsync(false),
window(nullptr),
frameRateChoice(0),
targetSpf(0.01600),
realSpf(0),
startClock(std::chrono::high_resolution_clock::now())
{
	if(SDL_Init(SDL_INIT_VIDEO))
	{
		std::cerr << SDL_GetError();
		throw std::runtime_error("Couldn't init SDL.");
	}
	
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	
	window = SDL_CreateWindow(
		"Frametool",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720,
		SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE|SDL_WINDOW_ALLOW_HIGHDPI);

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
	{
		SDL_GL_SetSwapInterval(1);
		vsync = SDL_GL_GetSwapInterval();
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.IniFilename = "frametool.ini";
	InitIni();

	ImGui_ImplSDL2_InitForOpenGL(window, glcontext);
	ImGui_ImplOpenGL3_Init("#version 330");

	mf.reset(new MainFrame);
	UpdateClientRect();
}

Window::~Window()
{
	mf.reset();
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	SDL_GL_DeleteContext(glcontext);
	SDL_DestroyWindow( window );
	SDL_Quit();
}

bool Window::PollEvents()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		ImGui_ImplSDL2_ProcessEvent(&event);
		switch(event.type)
		{
			case SDL_QUIT:
				mainWindow->wantsToClose = true;
				return true;
			case SDL_WINDOWEVENT:
				switch(event.window.event)
				{
					case SDL_WINDOWEVENT_SIZE_CHANGED:
						mf->SetClientRect(event.window.data1, event.window.data2);
						break;
					case SDL_WINDOWEVENT_CLOSE:
						return true;
				}
				break;
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				//GameLoopKeyHandle(event.key);
				break;
		}
	}

	return false;
}

void Window::UpdateClientRect()
{
	int w, h;
	SDL_GetWindowSize(window, &w, &h);
	mf->SetClientRect(w, h);
}

void Window::Render()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(window);
	ImGui::NewFrame();

	mf->Draw();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Window::SleepUntilNextFrame()
{
	if(!vsync)
	{
		std::chrono::duration<double> targetDur(targetSpf);
		std::chrono::duration<double> dur; 

		auto now = std::chrono::high_resolution_clock::now();
		if((dur = now - startClock) < targetDur)
		{
			std::this_thread::sleep_for((targetDur-dur));
		}
		//while( (dur = std::chrono::high_resolution_clock::now() - startClock) <= targetDur); 
	
		realSpf = dur.count();
	}
	else
	{
		realSpf = std::chrono::duration<double>(std::chrono::high_resolution_clock::now()-startClock).count();
	}
	startClock = std::chrono::high_resolution_clock::now();
}

void Window::SwapBuffers()
{
	SDL_GL_SwapWindow(window);
}

double Window::GetSpf()
{
	return realSpf;
}

void Window::ChangeFramerate()
{
	constexpr double framerateList[4] = {
		0.01600, 
		1.0/30.0, 
		1.0/10.0,
		1.0/2.0
	};

	frameRateChoice++;
	if(frameRateChoice >= 4)
		frameRateChoice = 0;
	targetSpf = framerateList[frameRateChoice];
}