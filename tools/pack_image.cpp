#include <iostream>
#include <image.h>
#include <string>

int main()
{
	std::string folder = "images/char/";

	std::string file = folder + "aka61_009.png";
	ImageData *image = new ImageData(file.c_str(), nullptr);
	
	std::cout << image->WriteAsPng("test.png");
	
	return 0;
}