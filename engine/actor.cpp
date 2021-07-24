#include "actor.h"

Actor::Actor(std::vector<Sequence> &sequences) :
sequences(sequences)
{}

void Actor::GotoSequence(int seq)
{
	if (seq < 0)
		return;

	currSeq = seq;
	currFrame = 0;
	GotoFrame(0);
	totalSubframeCount = 0;
}
