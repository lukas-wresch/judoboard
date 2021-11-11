#pragma once
#include <string>
#include "core.h"
#include "image.h"



typedef struct png_struct_def png_struct;
typedef struct png_info_def png_info;



namespace ZED
{
	class PNG : public Image
	{
	public:
		PNG() = default;
		DLLEXPORT PNG(const char* Filename);//Open PNG file from disk
		PNG(const std::string& Filename) : PNG(Filename.c_str()) {}

		//DLLEXPORT PNG(Image&& Img) : PNG(std::move((Blob)Img), Img.GetWidth(), Img.GetHeight(), Img.GetColorType()) {}
		PNG(Image&& Img) : Image(std::move(Img)) {}
		DLLEXPORT PNG(const ZED::Blob& Blob);//Open PNG data from memory
		PNG(ZED::Blob&& Blob) : PNG(Blob) {}//Open PNG data from memory

		DLLEXPORT PNG(ZED::Blob&  Blob, uint32_t Width, uint32_t Height, ZED::ColorType ColorType);//Opens raw image. Data is copied from the data blob
		DLLEXPORT PNG(ZED::Blob&& Blob, uint32_t Width, uint32_t Height, ZED::ColorType ColorType);//Opens raw image. Data is moved from the data blob

		DLLEXPORT static Blob Compress(const Image& Image);

		DLLEXPORT bool Save(const char* Filename);

	private:
		void Decompress(png_struct* png_ptr, png_info* info_ptr);
	};
}