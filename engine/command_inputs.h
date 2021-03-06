#ifndef INPUT_H_INCLUDED
#define INPUT_H_INCLUDED

#include <deque>
#include <string>
#include <map>
#include <filesystem>
#include <unordered_map>
#include <sol/sol.hpp>

typedef std::deque<uint32_t> input_deque;  //"Buffer" containing processed input.
constexpr size_t max_input_size = 32;

struct MotionData
{
	std::string motionStr; //Without the button press.
	sol::protected_function condition; //Without the button press.
	bool hasCondition = false;
	int startBuf = 0;
	int addBuf = 0;
	int seqRef = -1;
	int flags = 0;
	int priority = 0x7FFFFFFF; //Less is higher priority
};

class CommandInputs
{
	std::unordered_map<std::string, std::vector<MotionData>> motions;

public:
	struct CancelInfo
	{
		int subFrameCount;
		uint32_t actorFlags;
		bool canNormalCancel;
		bool canSpecialCancel;
		bool canInterrupt;
		int currentSequence;
	};

	CommandInputs();

	void LoadFromLua(std::filesystem::path defFile, sol::state &lua);

	//Returns sequence number and flags.
	MotionData ProcessInput(const input_deque &keyPresses, const char*motionType, int side, CancelInfo info);
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

public:
	enum //flags
	{
		neutralMove = 0x1,
		repeatable = 0x2,
		wipeBuffer = 0x4,
		interrupts = 0x8, //Will interrupt any move marked as interruptible.
		interruptible = 0x10,
		noCombo = 0x20
	};

};

int SanitizeKey(int lever);

#endif // INPUT_H_INCLUDED
