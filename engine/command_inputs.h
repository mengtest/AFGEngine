#ifndef INPUT_H_INCLUDED
#define INPUT_H_INCLUDED

#include <deque>
#include <string>
#include <map>
#include <filesystem>

typedef std::deque<uint32_t> input_deque;  //"Buffer" containing processed input.

struct MotionData
{
	std::string motionStr; //Without the button press.
	int bufLen = 0;
	int seqRef = 0;
	int flags;
};

class CommandInputs
{
	std::vector<MotionData> motions;

public:
	CommandInputs();

	void LoadFromLua(std::filesystem::path defFile);
	int GetSequenceFromInput(const input_deque &keyPresses, int side);
	void Charge(const input_deque &keyPresses);

private:
	bool MotionInput(const MotionData& md, const input_deque &keyPresses, int side);
	
	
	enum dir{
		up,
		down,
		left,
		right
	};
	input_deque chargeBuffer[4];
	int charges[4];
	int GetCharge(dir which, int frame, int side = 1 );

};

#endif // INPUT_H_INCLUDED
