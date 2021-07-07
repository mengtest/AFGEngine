#include "window.h"
#include <nfd.h>

int main(int, char**)
{
	Window window;
	if(NFD_Init() != NFD_OKAY)
		return 1;
		
	// Main loop
	while (window.PollEvents())
	{
		window.Render();
		window.SwapBuffers();
		window.SleepUntilNextFrame();
	}
	
	NFD_Quit();
	return 0;
}