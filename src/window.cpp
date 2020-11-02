#include "window.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>


#include <iostream>
#include <chrono>
#include "threading.h"

#include "main.h"
#include "shader.h"


bool fullscreen = false;
bool vsync = false;
bool shitty_drivers = false;

double realSpf = 0;

double targetSpf = 0.01666; //Duration of a frame in seconds.
const int internalWidth = 480;
const int internalHeight = 270;

GLFWwindow *mainWindow;

static void error_callback(int error, const char* description){
	std::cerr << error << " " << description << "\n";
}

static void window_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

GLFWwindow* InitWindow()
{
	//Initializing GLFW
	GLFWwindow* window;
	glfwSetErrorCallback(error_callback);
	if (!glfwInit())
		return 0;
	glfwWindowHint(GLFW_REFRESH_RATE, 60);

	// glfwWindowHint(GLFW_SAMPLES, 4);
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);

	if(fullscreen)
		window = glfwCreateWindow(mode->width, mode->height, "PNM", monitor, NULL);
	else
		window = glfwCreateWindow(internalWidth*2, internalHeight*2, "PNM: Plight", NULL, NULL);
		//window = glfwCreateWindow(internalWidth, internalHeight, "PNM: The unnameable fighting game", NULL, NULL);
	if (!window){
		glfwTerminate();
		return 0;
	}
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glfwMakeContextCurrent(window);

	glfwSetWindowSizeCallback(window, window_size_callback);
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	if(vsync)
		glfwSwapInterval(1); //vsync.
	//glfwSetCursorPos(window, 0, 0);

	if(glewInit()!=GLEW_OK)
	{
		std::cerr << "GLEW init failed.";
		return 0;
	}

	return window;
}

void SleepUntilNextFrame()
{
	if(!vsync)
	{
		if(shitty_drivers) //This is a workaround to some faulty vsync/sleep behaviour.
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
				threadNS::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
			while( (realSpf = glfwGetTime()) <= targetSpf);
		}
	}
	glfwSetTime(0);
}


void GlSetup2d()
{

	glEnable(GL_MULTISAMPLE  );

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, internalWidth, 0, internalHeight, 1, -1); //The difference should equal the borders of the viewport for pixel perfection.

	int width, height;
	glfwGetWindowSize(mainWindow, &width, &height);
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

	globalShaderProgram = LoadShader("vertex.txt","fragment.txt");
	glUseProgram(globalShaderProgram);
	glUniform1i(glGetUniformLocation(globalShaderProgram, "tex0"), 0 ); //Set texture unit to be accessed as 0.
}

float GetSpf()
{
	return realSpf;
}

const double framerateList[4] = {
	0.01666, 
	1.0/30.0, 
	1.0/10.0,
	1.0/2.0
};

void ChangeFramerate()
{
	static int which = 0;
	which++;
	if(which >= 4)
		which = 0;
	targetSpf = framerateList[which];
}