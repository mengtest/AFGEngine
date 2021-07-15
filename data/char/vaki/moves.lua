actTable = {
	{"UP",				7,		-1},
	{"UP_RIGHT",		5,		5},
	{"RIGHT",			1,		-1},
	{"DOWN_RIGHT",		3,		-1},
	{"DOWN",			3,		-1},
	{"DOWN_LEFT",		3,		-1},
	{"LEFT",			2,		-1},
	{"UP_LEFT",			11,		11},
	{"HIGH_PAIN",		14,		24},
	{"MID_PAIN",		15,		18},
	{"LOW_PAIN",		16,		-1},
	{"A5",				17,		35},
	{"A2",				13,		35},
	{"A6",				17,		35},
	{"B5",				-1,		-1},
	{"B2",				-1,		-1},
	{"B6",				-1,		-1},
	{"B4",				-1,		-1},
	{"B3",				-1,		-1},
	{"C5",				-1,		-1},
	{"C2",				-1,		-1},
	{"C6",				-1,		-1},
	{"C4",				-1,		-1},
	{"C3",				-1,		-1},
	{"D5",				-1,		-1},
	{"D2",				-1,		-1},
	{"_U1",				-1,		-1},
	{"GUARD4",			40,		-1},
	{"GUARD1",			41,		-1},
	{"GUARD4_STARTUP",	31,		-1},
	{"GUARD5_STARTUP",	32,		-1},
	{"STAND_180",		9,		-1},
	{"CROUCH_180",		12,		-1},
	{"STAND_UP",		4,		-1},
	{"BUNKER",			-1,		-1},
	{"TRAIT",			-1,		-1},
	{"TECHING",			-1,		-1},
	{"_U3",				-1,		-1},
	{"_U4,",			-1,		-1}
}
actTable[0] = {"NEUTRAL", 0, -1}

inputs = {
	default = {
		{input = "5656", blen = 12, ref = 25},
		{input = "5454", blen = 12, ref = 26},
		{input = "623A", blen = 15, ref = 34}
	},
	air = {
		{input = "5R56", blen = 14, ref = 27},
		{input = "5L54", blen = 14, ref = 28}
	}
}
