#include <args.hxx>
#include <squish.h>
#include <lz4.h>
#include <image.h>
#include <memory>
#include <fstream>
#include <filesystem>
#include <cmath>

int main(int argc, char **argv)
{
	args::ArgumentParser parser("lzs3 image compressor.",
	"Encodes a single image into a lz4-compressed s3tc texture format, thus lzs3. "
	"The input has to be a PNG file.");
	args::Positional<std::string> srcImage(parser, "FILE", "Path to the image.");
	args::HelpFlag help(parser, "help", "Display this help menu.", {'h', "help"});
	args::ValueFlag<std::string> outName(parser, "name", "Output filename. Defaults to [FILE].", {'o'});
	args::ValueFlag<int> dtxn(parser, "format",
		"DTXn format.\n"
		"0 - Use DXT1A compression. (Unsupported)\n"
		"1 - Use DXT1 compression <- Default\n"
		"3 - Use DXT3 compression\n"
		"5 - Use DXT5 compression",
		{'x', "dtx"}, 1);
	args::ValueFlag<int> quality(parser, "quality",
		"The quality of the S3 compression.\n"
		"0 - No compression. (Unsupported)\n"
		"1 - Low / fast <- Default\n"
		"2 - High / slow\n"
		"3 - Best / very slow",
		{'q', "quality"}, 1);
	args::Flag weightAlpha(parser, "weight", "Weight colour by alpha. Has no effect on low quality compression.", {'w',"weight"});
	args::ValueFlag<float> alphaCorrect(parser, "float", "Exponentiate alpha to this power. Useful to get rid of funny borders.", {'a',"alpha"});
	try
	{
		parser.ParseCLI(argc, argv);
		if(!srcImage)
		{
			std::cout << "No image file selected. Use -h for help.";
			return 0;
		}
	}
	catch (const args::Help&)
	{
		std::cout << parser;
		return 0;
	}
	catch (const args::ParseError& e)
	{
		std::cerr << e.what() << std::endl;
		std::cerr << parser;
		return 1;
	}
	catch (args::ValidationError e)
	{
		std::cerr << e.what() << std::endl;
		std::cerr << parser;
		return 1;
	}

	using namespace squish;

	//Parse command line
	int format = 0;
	const std::vector<std::pair<int,int>> types = {
		{1, kDxt1},
		{3, kDxt3},
		{5, kDxt5}
	};
	for(auto &t : types)
	{
		if(dtxn.Get() == t.first)
		{
			format = t.second;
			break;
		}
	}
	if(format == 0)
	{
		std::cerr << "Invalid DTXn format.";
		return 0;
	}

	int fit = 0;
	const std::vector<std::pair<int,int>> qualities = {
		{1, kColourRangeFit},
		{2, kColourClusterFit},
		{3, kColourIterativeClusterFit}
	};
	for(auto &q : qualities)
	{
		if(quality.Get() == q.first)
		{
			fit = q.second;
			break;
		}
	}
	if(fit == 0)
	{
		std::cerr << "Invalid quality number.";
		return 0;
	}

	int extra = 0;
	if(weightAlpha)
		extra = kWeightColourByAlpha;

	std::filesystem::path filepath = srcImage.Get();

	//Compress image
	ImageData img(filepath.string().c_str());

	if(alphaCorrect && img.bytesPerPixel == 4)
	{
		float exponent = alphaCorrect.Get();
		for(int i = 0; i < img.height*img.width*4; i+=4)
		{
			auto& alpha = img.data[i+3];
			alpha = pow((float)alpha/255.f, exponent) * 255.f;
		}
	}

	int flags = format | fit | extra;
	int size = GetStorageRequirements( img.width, img.height, flags );
	auto dxtData = std::make_unique<char[]>(size);
	CompressImage(img.data, img.width, img.height, dxtData.get(), flags );

	int cSize = LZ4_compressBound(size);
	auto compressed = std::make_unique<char[]>(cSize);

	uint32_t outBytes = LZ4_compress_default(dxtData.get(), compressed.get(), size, cSize);

	struct{
		uint32_t type;
		uint32_t size;
		uint32_t cSize;
		uint16_t w,h;
	} meta;
	meta.size = size;
	meta.cSize = outBytes;
	meta.w = img.width;
	meta.h = img.height;
	if(format & kDxt1)
		meta.type = 1;
	else if(format & kDxt3)
		meta.type = 2;
	else if(format & kDxt5)
		meta.type = 3;

	std::filesystem::path outFilepath = filepath.parent_path()/filepath.stem();
	if(outName)
		outFilepath = outName.Get();
	outFilepath += ".lzs3";
	std::ofstream file(outFilepath, std::ios_base::binary);
	file.write((char*)&meta, sizeof(meta));
	file.write(compressed.get(), outBytes);

	/* ImageData outImg(img.width, img.height, 4);
	DecompressImage(outImg.data, img.width, img.height, dxtData.get(), format );
	outImg.WriteAsPng("test.png");
	outImg.data = nullptr;
	*/
}