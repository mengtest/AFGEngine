#ifndef WINDOW_H_INCLUDED
#define WINDOW_H_INCLUDED

#include <GL/glew.h>
#include <GLFW/glfw3.h>


extern bool fullscreen;
extern bool vsync;
extern const int internalWidth;
extern const int internalHeight;

extern GLFWwindow *mainWindow;


GLFWwindow* InitWindow(); //Sets up initial window size, opengl configuration and shaders.


//Sleeps until it's time to process the next frame.
void SleepUntilNextFrame();

void GlSetup2d();

float GetSpf();

#endif // WINDOW_H_INCLUDED
