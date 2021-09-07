texture = {
	width = 322,
	height = 285,
	rects = {
		{
			name = "topbar.png",
			coords = {
				x = 1,
				y = 1,
				w = 320,
				h = 78
			}
		},
--[[ 		{
			name = "portrait1.png",
			coords = {
				x = 1,
				y = 81,
				w = 144,
				h = 120
			}
		},
		{
			name = "portrait2.png",
			coords = {
				x = 147,
				y = 81,
				w = 144,
				h = 120
			}
		}, ]]
		{
			name = "meterbase.png",
			coords = {
				x = 1,
				y = 203,
				w = 238,
				h = 45
			}
		},
		{
			name = "metermask.png",
			coords = {
				x = 1,
				y = 250,
				w = 192,
				h = 21
			}
		},
		{
			name = "guardmask.png",
			coords = {
				x = 195,
				y = 250,
				w = 120,
				h = 22
			}
		},
		{
			name = "gauge.png",
			coords = {
				x = 1,
				y = 274,
				w = 212,
				h = 10
			},
			isBar = true
		},
		{
			name = "text.png",
			coords = {
				x = 215,
				y = 274,
				w = 97,
				h = 9
			},
			pos = {y=8},
			mirror = false,
			fromBottom = true,
		},
		{
			name = "text.png",
			coords = {
				x = 215,
				y = 274,
				w = 97,
				h = 9
			},
			pos = {y=8},
			mirror = false,
			fromRight = true,
			fromBottom = true,
		}
	},
	file = "hud.lzs3"
}

names = {}
for i, v in ipairs(texture.rects) do
	names[v.name] = i
end

texture.scale = 0.75;
texture.rects[names["meterbase.png"]].fromBottom = true;
texture.rects[names["metermask.png"]].fromBottom = true;
texture.rects[names["metermask.png"]].isBar = true;
texture.rects[names["metermask.png"]].pos = {x = 35, y = 21};
texture.rects[names["guardmask.png"]].pos = {x = 155, y = -25};
texture.rects[names["guardmask.png"]].isBar = true;
texture.rects[names["gauge.png"]].isBar = true;
texture.rects[names["gauge.png"]].pos = {x = 61, y = -9};