#include "render_context.h"
#include "shader.h"

#include <glad/glad.h>
#include <SDL.h>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <stdexcept>


RenderContext::RenderContext() :
view(1), model(1), initialized(false)
{}

RenderContext::RenderContext(SDL_Window *window) :RenderContext()
{
	SetupGl(window);
}

RenderContext::~RenderContext()
{
	if(initialized)
		glDeleteProgram(currentProgram);
	SDL_GL_DeleteContext(glcontext);
}

void RenderContext::SetupGl(SDL_Window *window)
{
	glcontext = SDL_GL_CreateContext(window);
	if (!gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress))
	{
		SDL_DestroyWindow( window );
		SDL_Quit();
		throw std::runtime_error("Glad couldn't initialize OpenGL context.");
	}

	std::cout << "OpenGL " << GLVersion.major <<"."<< GLVersion.minor <<"\n";

	projection = glm::ortho<float>(0, internalWidth, 0, internalHeight, -1, 1);

	int width, height;
	SDL_GetWindowSize(window, &width, &height);
	UpdateViewport(width, height);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	currentProgram = LoadShader("data/def.vert", "data/def.frag");
	glUseProgram(currentProgram);
	glUniform1i(glGetUniformLocation(currentProgram, "tex0"), 0 ); //Set texture unit to be accessed as 0.
	transformLoc = glGetUniformLocation(currentProgram, "transform");
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(projection));

	initialized = true;
}

void RenderContext::UpdateViewport(float width, float height)
{
	constexpr float asRatio = (float)internalWidth/(float)internalHeight;
	if(width/height > asRatio) //wider
	{
		glViewport((width-height*asRatio)/2, 0, height*asRatio, height);
	}
	else
	{
		glViewport(0, (height-width/asRatio)/2, width, width/asRatio);
	}
}

void RenderContext::SetModelView(glm::mat4 _view)
{
	view = _view;
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(projection * view));
}