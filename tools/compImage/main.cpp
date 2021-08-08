#include <squish.h>
#include <image.h>
#include <memory>
#include <lz4.h>
#include <fstream>

int main()
{
   using namespace squish;
   ImageData img("data/images/background.png");
   int format = kDxt1;
   int fit = kColourRangeFit;
   int extra = 0;// kWeightColourByAlpha;
   if(img.bytesPerPixel == 4)
      format = kDxt1;
   
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

   std::ofstream file("data/images/background.lzs3", std::ios_base::binary);
   file.write((char*)&meta, sizeof(meta));
   file.write(compressed.get(), outBytes);
   /* ImageData outImg(img.width, img.height, 4);
   DecompressImage(outImg.data, img.width, img.height, dxtData.get(), format );
   outImg.WriteAsPng("test.png");
   outImg.data = nullptr;
   */
}