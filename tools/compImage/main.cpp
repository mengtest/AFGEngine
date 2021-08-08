#include <squish.h>
#include <image.h>
#include <memory>

int main()
{
   using namespace squish;
   ImageData img("data/images/background.png");
   int format = kDxt1;
   int fit = kColourClusterFit;
   int extra = 0;// kWeightColourByAlpha;
   if(img.bytesPerPixel == 4)
      format = kDxt5;
   
   int flags = format | fit | extra;
   int size = GetStorageRequirements( img.width, img.height, flags );
   auto dxtData = std::make_unique<uint8_t[]>(size);
   CompressImage(img.data, img.width, img.height, dxtData.get(), flags );
   
   ImageData outImg(img.width, img.height, 4);
   DecompressImage(outImg.data, img.width, img.height, dxtData.get(), format );

   outImg.WriteAsPng("tes1.png");
   outImg.data = nullptr;
}