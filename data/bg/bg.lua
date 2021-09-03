stage = {
	scale = 0.5,
	width = 600,
	height = 450,
	layers = {
		{ --Lowermost layer
			y = 160,
			xParallax = 0.3,
			yParallax = 0.7,
			elements = {
				{id = 2}
			}
		},
		{
			y = 0,
			xParallax = 0.8,
			yParallax = 0.8,
			elements = {
				{id = 1}
			}
		},
 		{ --Uppermost layer.
			xParallax = 1,
			elements = {
				{id = 0}
			}
		}
	}
}

graphics = {
	{
		type = lzs3,
		image = "bg.lzs3",
		vertex = "bg.vt4",
		filter = true,
	}
}