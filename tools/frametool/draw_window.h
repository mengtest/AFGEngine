#ifndef DRAWWINDOW_H_GUARD
#define DRAWWINDOW_H_GUARD

struct FrameState
{
	int seq;
	int frame;
};
//ImGui Windows that draw themselves. Just for utility.
class DrawWindow
{
public:
	DrawWindow(FrameState &state):
		cs(state){};

	FrameState &cs; //Current state.
	bool isVisible = true;

	virtual void Draw() = 0;
protected:

};

#endif /* DRAWWINDOW_H_GUARD */
