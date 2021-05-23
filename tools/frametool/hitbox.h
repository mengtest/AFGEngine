#ifndef HITBOX_H_GUARD
#define HITBOX_H_GUARD
#include <vector>

struct Hitbox
{
	int xy[4];
};

using BoxList = std::vector<Hitbox>;

#endif /* HITBOX_H_GUARD */
