#include "render_context.h"
#include <shader.h>

#include <glad/glad.h>
#include <SDL.h>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <stdexcept>


RenderContext::RenderContext() :
projection(1),
initialized(false),
uniforms("Common", 1)
{}

RenderContext::RenderContext(SDL_Window *window) :RenderContext()
{
	SetupGl(window);
}

RenderContext::~RenderContext()
{
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

	projection = glm::ortho<float>(0, internalWidth, 0, internalHeight, -32768, 32767);

	int width, height;
	SDL_GetWindowSize(window, &width, &height);
	UpdateViewport(width, height);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	defaultS.LoadShader("data/def.vert", "data/def.frag");
	defaultS.Use();	

	//Bind transform matrix uniforms.
	uniforms.Init(sizeof(float)*16);
	uniforms.Bind(defaultS.program);
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

void RenderContext::SetModelView(glm::mat4 view)
{
	view = projection*view;
	uniforms.SetData(glm::value_ptr(view));
}

void RenderContext::PushShaderUboBind(Shader *shader)
{
	uniforms.Bind(shader->program);
}

void RenderContext::SetShader(int type)
{
	switch(type)
	{
	case DEFAULT:
		defaultS.Use();
		break;
	default:
		std::cerr << __FUNCTION__ << ": No such shader type.\n";
	}
}
