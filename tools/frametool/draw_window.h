#ifndef DRAWWINDOW_H_GUARD
#define DRAWWINDOW_H_GUARD

#include "render.h"
#include "framedata.h"

struct FrameState
{
	int seq;
	int frame;
};
//ImGui Windows that draw themselves. Just for utility.
class DrawWindow
{
public:
	DrawWindow(Render* render, Framedata *frameData, FrameState &state):
		cs(state),
		render(render),
		frameData(frameData){};

	FrameState &cs; //Current state.
	Render* render;
	Framedata *frameData;
	bool isVisible = true;

	virtual void Draw() = 0;
protected:

};

#endif /* DRAWWINDOW_H_GUARD */
