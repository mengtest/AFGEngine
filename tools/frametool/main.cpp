#include "window.h"

int main(int, char**)
{
	Window window;
	// Main loop
	while (window.PollEvents())
	{
		window.Render();
		window.SwapBuffers();
		window.SleepUntilNextFrame();
	}
	
	return 0;
}