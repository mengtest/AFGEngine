#ifndef RENDER_CONTEXT_H_GUARD
#define RENDER_CONTEXT_H_GUARD
#include <SDL.h>
#include <glm/mat4x4.hpp>

constexpr int internalWidth = 480;
constexpr int internalHeight = 270;

class RenderContext
{
private:
	SDL_GLContext glcontext;
	glm::mat4 projection, view, model;
	int transformLoc;

	unsigned int currentProgram;
	bool initialized;

public:
	RenderContext();
	RenderContext(SDL_Window *window);
	~RenderContext();

	void SetupGl(SDL_Window *window);
	void SetModelView(glm::mat4 view = glm::mat4(1));
	void UpdateViewport(float width, float height);
};

#endif /* RENDER_CONTEXT_H_GUARD */
