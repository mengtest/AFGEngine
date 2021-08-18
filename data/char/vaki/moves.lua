local f = {
	neutral 		= 1 << 0,
	repeatable 		= 1 << 1,
	wipeBuffer 		= 1 << 2,
	interrupts 		= 1 << 3,
	interruptible	= 1 << 4,
	noCombo			= 1 << 5,
}

inputs = {
	ground = {

			
		{input = "23~6b", buf = 8, ref = 507},
		{input = "23~6a", buf = 8, ref = 506},

		--Power dunk
		{input = "62~3c", buf = 8, ref = 207},
		{input = "62~3b", buf = 8, ref = 206},
		{input = "62~3a", buf = 8, ref = 554},
		
		--Tsuki
		{input = "21~4a", buf = 8, ref = 180},
		--{input = "\030R/~4a", buf = 8, ref = 155},

		{input = "9", buf = 1, ref = 35, flag = f.repeatable},
		{input = "7", buf = 1, ref = 37, flag = f.repeatable},
		{input = "8", buf = 1, ref = 36, flag = f.repeatable},
				
		{input = "5454", buf = 8, ref = 45, flag = f.noCombo},
		{input = "5656", buf = 8, ref = 43, flag = f.noCombo},
		{input = "~4ab", buf = 1, ref = 45, flag = f.interrupts | f.noCombo},
		{input = "~6ab", buf = 1, ref = 43, flag = f.interrupts | f.noCombo},
		
		{input = "~3c", buf = 1, ref = 445, flag = f.interruptible},
		{input = "~6c", buf = 2, ref = 60, flag = f.interruptible},
		{input = "~4c", buf = 2, ref = 80, flag = f.interruptible},
		{input = "~Dc", buf = 2, ref = 6, flag = f.interruptible},
		{input = "~Db", buf = 2, ref = 5, flag = f.interruptible},
		{input = "~Da", buf = 2, ref = 4, flag = f.interruptible | f.repeatable},
		{input = "~c", buf = 2, ref = 3, flag = f.interruptible},
		{input = "~b", buf = 2, ref = 2, flag = f.interruptible},
		{input = "~a", buf = 2, ref = 1, flag = f.interruptible | f.repeatable},
		
		{input = "6", buf = 1, ref = 10, flag = f.neutral},
		{input = "4", buf = 1, ref = 11, flag = f.neutral},
		{input = "D", buf = 1, ref = 12, flag = f.neutral, cond = C_toCrouch},
	},
	air = {
		{input = "9", buf = 1, ref = 38, cond = C_notHeldFromJump},
		{input = "7", buf = 1, ref = 40, cond = C_notHeldFromJump},
		{input = "8", buf = 1, ref = 39, cond = C_notHeldFromJump},
		{input = "5L54", buf = 8, ref = 47, flag = f.repeatable | f.wipeBuffer | f.noCombo, cond = C_heightRestriction},
		{input = "5R56", buf = 8, ref = 46, flag = f.repeatable | f.wipeBuffer, cond = C_heightRestriction},
		{input = "~4ab", buf = 1, ref = 47, flag = f.repeatable | f.interrupts | f.noCombo, cond = C_heightRestriction},
		{input = "~6ab", buf = 1, ref = 46, flag = f.repeatable | f.interrupts, cond = C_heightRestriction},
		{input = "~c", buf = 2, ref = 9, flag = f.interruptible},
		{input = "~b", buf = 2, ref = 8, flag = f.interruptible},
		{input = "~a", buf = 2, ref = 7, flag = f.interruptible | f.repeatable},
		
	}
}
