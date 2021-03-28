#ifndef RENDER_CONTEXT_H_GUARD
#define RENDER_CONTEXT_H_GUARD
#include "shader.h"
#include "ubo.h"
#include <SDL.h>
#include <glm/mat4x4.hpp>

constexpr int internalWidth = 480;
constexpr int internalHeight = 270;

class RenderContext
{
private:
	SDL_GLContext glcontext;
	glm::mat4 projection;
	
	bool initialized;

	Shader defaultS;
	Shader indexedS;
	Ubo uniforms;

	int paletteSlotL;

public:
	RenderContext();
	RenderContext(SDL_Window *window);
	~RenderContext();

	void SetupGl(SDL_Window *window);
	void SetModelView(glm::mat4 view = glm::mat4(1));
	void UpdateViewport(float width, float height);

	//Bad idea maybe
	void SetShader(int type);
	enum{
		DEFAULT,
		PALETTE
	};

	//TODO: move it elsewhere
	void SetPaletteSlot(int slot);
};

#endif /* RENDER_CONTEXT_H_GUARD */
