#ifndef WINDOW_H_INCLUDED
#define WINDOW_H_INCLUDED

#include <SDL.h>
#include <chrono>

constexpr int internalWidth = 480;
constexpr int internalHeight = 270;

class Window
{
public:
	SDL_GLContext glcontext = nullptr;
	bool wantsToClose;
	
private:
	bool fullscreen;
	bool vsync;
	bool busyWait;
	SDL_Window* window;

	int frameRateChoice;
	double targetSpf;
	double realSpf;
	std::chrono::time_point<std::chrono::high_resolution_clock> startClock; 

	void SetupGl(SDL_Window *window);
public:
	Window();
	~Window();

	void SwapBuffers();
	void ChangeFramerate();

	//Sleeps until it's time to process the next frame.
	void SleepUntilNextFrame();

	double GetSpf();
	void UpdateViewport(float width, float height);
};

extern Window *mainWindow;

#endif // WINDOW_H_INCLUDED
