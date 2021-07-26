local f = {
	neutral 		= 1 << 0,
	repeatable 		= 1 << 1,
	wipeBuffer 		= 1 << 2,
	interrupts 		= 1 << 3,
	interrumpible	= 1 << 4,
}

inputs = {
	ground = {
		{input = "23~6b", buf = 8, ref = 507},
		{input = "23~6a", buf = 8, ref = 506},
		{input = "21~4a", buf = 8, ref = 180},
		--{input = "\030R/~4a", buf = 8, ref = 155},

		{input = "7", buf = 1, ref = 37, flag = f.repeatable},
		{input = "8", buf = 1, ref = 36, flag = f.repeatable},
		{input = "9", buf = 1, ref = 35, flag = f.repeatable},
		
		{input = "5454", buf = 8, ref = 45},
		{input = "5656", buf = 8, ref = 43},
		{input = "~4ab", buf = 1, ref = 45, flag = f.interrupts},
		{input = "~6ab", buf = 1, ref = 43, flag = f.interrupts},
		
		{input = "~3c", buf = 1, ref = 445, flag = f.interrumpible},
		{input = "~6c", buf = 2, ref = 60, flag = f.interrumpible},
		{input = "~4c", buf = 2, ref = 80, flag = f.interrumpible},
		{input = "~Dc", buf = 2, ref = 6, flag = f.interrumpible},
		{input = "~Db", buf = 2, ref = 5, flag = f.interrumpible},
		{input = "~Da", buf = 2, ref = 4, flag = f.interrumpible},
		{input = "~c", buf = 2, ref = 3, flag = f.interrumpible},
		{input = "~b", buf = 2, ref = 2, flag = f.interrumpible},
		{input = "~a", buf = 2, ref = 1, flag = f.interrumpible},
		
		{input = "6", buf = 1, ref = 10, flag = f.neutral},
		{input = "4", buf = 1, ref = 11, flag = f.neutral},
		{input = "D", buf = 1, ref = 12, flag = f.neutral, cond = C_toCrouch},
	},
	air = {
		{input = "U!7", buf = 1, ref = 40},
		{input = "U!8", buf = 1, ref = 39},
		{input = "U!9", buf = 1, ref = 38},
		{input = "5L54", buf = 8, ref = 47, flag = f.repeatable | f.wipeBuffer, cond = C_heightRestriction},
		{input = "5R56", buf = 8, ref = 46, flag = f.repeatable | f.wipeBuffer, cond = C_heightRestriction},
		{input = "~4ab", buf = 1, ref = 47, flag = f.repeatable | f.interrupts, cond = C_heightRestriction},
		{input = "~6ab", buf = 1, ref = 46, flag = f.repeatable | f.interrupts, cond = C_heightRestriction},
		{input = "~c", buf = 2, ref = 9, flag = f.interrumpible},
		{input = "~b", buf = 2, ref = 8, flag = f.interrumpible},
		{input = "~a", buf = 2, ref = 7, flag = f.interrumpible},
		
	}
}
