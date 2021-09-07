#include "util.h"

int DrawText(std::string text, std::vector<float> &arr, float x, float y)
{
	int i = 0;
	while (i < text.size() && i < arr.size()/24){
			int character = (text[i]);
			constexpr int rows = 16, cols = 16;
			const float magicxoffset = (1.f/rows);
			const float magicyoffset = (1.f/cols);
			constexpr float width = 4, height = 8; 

			//BL
			arr[i*24 + 0] = 0+x+(i*width);
			arr[i*24 + 1] = y;
			arr[i*24 + 2] = magicxoffset*((character%rows));
			arr[i*24 + 3] = 1-(magicyoffset*(character/rows));

			//BR
			arr[i*24 + 4] = width+x+(i*width);
			arr[i*24 + 5] = y;
			arr[i*24 + 6] = magicxoffset*((character%rows)+1);
			arr[i*24 + 7] = 1-(magicyoffset*(character/rows));

			//TR
			arr[i*24 + 8] = width+x+(i*width);
			arr[i*24 + 9] = y-height;
			arr[i*24 + 10] = magicxoffset*((character%rows)+1);
			arr[i*24 + 11] = 1-(magicyoffset*((character/rows)+1));

			//SPLIT
			//TR
			arr[i*24 + 12] = width+x+(i*width);
			arr[i*24 + 13] = y-height;
			arr[i*24 + 14] = magicxoffset*((character%rows)+1);
			arr[i*24 + 15] = 1-(magicyoffset*((character/rows)+1));

			//TL
			arr[i*24 + 16] = 0+x+(i*width);
			arr[i*24 + 17] = y-height;
			arr[i*24 + 18] = magicxoffset*((character%rows));
			arr[i*24 + 19] = 1-(magicyoffset*((character/rows)+1));

			//TL
			arr[i*24 + 20] = 0+x+(i*width);
			arr[i*24 + 21] = y;
			arr[i*24 + 22] = magicxoffset*((character%rows));
			arr[i*24 + 23] = 1-(magicyoffset*(character/rows));

			++i;
	}

	return i*6;
}