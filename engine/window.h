#ifndef WINDOW_H_INCLUDED
#define WINDOW_H_INCLUDED

#include <glad/glad.h>
#include <SDL.h>

constexpr int internalWidth = 480;
constexpr int internalHeight = 270;

class Window
{
public:
	bool wantsToClose;
	bool fullscreen;
	bool vsync;
	bool busyWait;

private:
	SDL_GLContext glcontext;
	SDL_Window* window;

	int frameRateChoice;
	double targetSpf;
	double realSpf;

public:
	Window();
	~Window();

	void SwapBuffers();
	void ChangeFramerate();

	//Sleeps until it's time to process the next frame.
	void SleepUntilNextFrame();

	void GlSetup2d();

	double GetSpf();
};

extern Window *mainWindow;

#endif // WINDOW_H_INCLUDED
