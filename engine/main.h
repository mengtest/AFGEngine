#ifndef MAIN_H3_INCLUDED
#define MAIN_H3_INCLUDED


/*enum //menu static texture list
{
    T_ELEMENTS,
    T_FONTBIG,
    T_FONTSMALL,
    T_BACKGROUND1,
    T_BACKGROUND2
};*/

struct VertexData8
{
	unsigned short x,y,s,t;
	unsigned short atlasId;
};

enum //gamestate
{
    GS_MENU,
    GS_CHARSELECT,
    GS_PLAY
};

enum //ingame stage static texture list. Most probably will get replaced by something else.
{
    T_STAGELAYER1, //multiple layers on different textures?
    T_HUD,
    T_FONT,
    T_CHAR
};

void PlayLoop();
void EventLoop();

#endif // MAIN_H_INCLUDED
