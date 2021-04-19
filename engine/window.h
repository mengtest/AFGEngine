#ifndef WINDOW_H_INCLUDED
#define WINDOW_H_INCLUDED

#include "render_context.h"
#include <SDL.h>
#include <chrono>

class Window
{
public:
	RenderContext context;
	bool wantsToClose;
	bool fullscreen;
	bool vsync;
	bool busyWait;

private:
	SDL_Window* window;

	int frameRateChoice;
	double targetSpf;
	double realSpf;
	std::chrono::time_point<std::chrono::high_resolution_clock> startClock; 

	
public:
	Window();
	~Window();

	void SwapBuffers();
	void ChangeFramerate();

	//Sleeps until it's time to process the next frame.
	void SleepUntilNextFrame();

	double GetSpf();
};

extern Window *mainWindow;

#endif // WINDOW_H_INCLUDED
