#include "../include/png.h"
#include "../include/file.h"
#include "../include/log.h"
//#include "../external/libpng-1.6.37/png.h"
#include <png.h>


using namespace ZED;


//#pragma comment(lib, "../external/libpng-1.6.37/projects/vstudio/Release Library/libpng16.lib")
//#pragma comment(lib, "../external/libpng-1.6.37/projects/vstudio/Release Library/zlib.lib")
#pragma comment(lib, "libpng16.lib")



PNG::PNG(const char* Filename)
{
    FILE* file = nullptr;
    fopen_s(&file, Filename, "rb");

    Log::Debug("Opening file " + std::string(Filename));

    if (!file)
    {
        Log::Error("Could not open file: " + std::string(Filename));
        return;
    }

    png_struct* png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!png_ptr)
    {
        Log::Error("png_create_read_struct() failed");
        fclose(file);
        return;
    }

    png_info* info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        Log::Error("png_create_info_struct() failed");
        fclose(file);
        png_destroy_read_struct(&png_ptr, nullptr, nullptr);
        return;
    }

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        Log::Error("Error in libpng occured");
        fclose(file);
        png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
        return;
    }

    png_init_io(png_ptr, file);

    Decompress(png_ptr, info_ptr);

    fclose(file);
}



PNG::PNG(const ZED::Blob& Blob)
{
    if (Blob.GetSize() < 256)
        return;

    png_struct* png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!png_ptr)
    {
        Log::Error("png_create_read_struct() failed");
        return;
    }

    png_info* info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        Log::Error("png_create_info_struct() failed");
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        return;
    }

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        Log::Error("Error in libpng occured");
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return;
    }

    png_set_read_fn(png_ptr, const_cast<void*>((const void*)Blob), [](png_struct* ps, png_byte* Data, png_size_t Length) {
        ZED::Blob* b = (ZED::Blob*)png_get_io_ptr(ps);
        png_size_t bytes_read = b->OutputTo(Data, Length);

        if (bytes_read != Length)
            png_error(ps, "Read Error!");
    });

    png_init_io(png_ptr, (FILE*)(const void*)Blob);
    png_set_sig_bytes(png_ptr, 8);
    
    Decompress(png_ptr, info_ptr);
}



PNG::PNG(ZED::Blob& Blob, uint32_t Width, uint32_t Height, ZED::ColorType ColorType) : Image(Width, Height, ColorType)
{
    m_ImageData = ZED::Blob(Width * Height * ColorTypeSize(ColorType));

    m_ImageData << Blob;
}



PNG::PNG(ZED::Blob&& Blob, uint32_t Width, uint32_t Height, ZED::ColorType ColorType) :
    Image(std::move(Blob), Width, Height, ColorType)
{
}



void PNG::Decompress(png_struct* png_ptr, png_info* info_ptr)
{
    Log::Debug("Decompressing PNG data");

    png_read_info(png_ptr, info_ptr);

    int bit_depth, color_type;
    png_get_IHDR(png_ptr, info_ptr, &m_Width, &m_Height, &bit_depth, &color_type, nullptr, nullptr, nullptr);

    if (color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_expand(png_ptr);//Expand to RGB24
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
        png_set_expand(png_ptr);//Expand to 8 bit data
    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
        png_set_expand(png_ptr);//Expand alpha channel


    m_ColorType = ColorType::R8G8B8A8;


    png_read_update_info(png_ptr, info_ptr);

    size_t rowbytes = png_get_rowbytes(png_ptr, info_ptr);
    int channels = (int)png_get_channels(png_ptr, info_ptr);

    m_ImageData = ZED::Blob(rowbytes * m_Height);

    png_bytep* row_pointers = new png_bytep[m_Height];//Pointers to pointers

    for (uint32_t i = 0; i < m_Height; i++)
        row_pointers[i] = m_ImageData + i * rowbytes;

    //Read image
    png_read_image(png_ptr, row_pointers);

    delete[] row_pointers;

    png_read_end(png_ptr, nullptr);

    png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);

    Log::Debug("Decompressed " + std::to_string(m_Width)  + "x" + std::to_string(m_Height)  + " pixels");
}



Blob PNG::Compress(const Image& Image)
{
    Blob ret;
    png_struct* writeStruct = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);

    if (!writeStruct)
    {
        Log::Error("png_create_write_struct() failed");
        return ret;
    }

    png_info* infoStruct = png_create_info_struct(writeStruct);
    if (!infoStruct)
    {
        Log::Error("png_create_info_struct() failed");
        return ret;
    }

    if (setjmp(png_jmpbuf(writeStruct)))
    {
        Log::Error("Error in libpng occured");
        png_free_data(writeStruct, infoStruct, PNG_FREE_ALL, -1);
        png_destroy_write_struct(&writeStruct, &infoStruct);
        return ret;
    }

    //png_init_io(writeStruct, (FILE*)(const void*)ret);
    png_set_write_fn(writeStruct, &ret, [](png_structp ps, png_bytep data, png_size_t length) {
        ZED::Blob* b = (ZED::Blob*)png_get_io_ptr(ps);
        b->Append(data, length);
        }, [](png_structp png_ptr) {});

    //Write header (8 bit color depth)
    png_set_IHDR(writeStruct, infoStruct, Image.GetWidth(), Image.GetHeight(), 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    png_write_info(writeStruct, infoStruct);

    //RGB 3-byte stride.
    const uint32_t rowStride = Image.GetWidth() * 3;
    for (uint32_t row = 0; row < Image.GetHeight(); row++)
    {
        uint32_t rowOffset = row * rowStride;
        png_write_row(writeStruct, &Image[rowOffset]);
    }

    png_write_end(writeStruct, infoStruct);

    png_free_data(writeStruct, infoStruct, PNG_FREE_ALL, -1);
    png_destroy_write_struct(&writeStruct, &infoStruct);
    return ret;
}



bool PNG::Save(const char* Filename)
{
    ZED::File file(Filename, true);

    if (!file)
    {
        Log::Error("Could not open file: " + std::string(Filename));
        return false;
    }

    png_struct* writeStruct = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);

    if (!writeStruct)
    {
        Log::Error("png_create_write_struct() failed");
        return false;
    }

    png_info* infoStruct = png_create_info_struct(writeStruct);
    if (!infoStruct)
    {
        Log::Error("png_create_info_struct() failed");
        return false;
    }

    if (setjmp(png_jmpbuf(writeStruct)))
    {
        Log::Error("Error in libpng occured");
        png_free_data(writeStruct, infoStruct, PNG_FREE_ALL, -1);
        png_destroy_write_struct(&writeStruct, &infoStruct);
        return false;
    }

    png_init_io(writeStruct, file.GetFileHandle());

    //Write header (8 bit color depth)
    png_set_IHDR(writeStruct, infoStruct, m_Width, m_Height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    //Set title
    /*if (title != nullptr && strlen(title) > 0)
    {
        std::array<char, 50> mutableKey = {};
        strcpy(mutableKey.data(), "Title");
        std::array<char, 200> mutableText = {};
        strncpy(mutableText.data(), title, 199);

        png_text titleText;
        titleText.compression = PNG_TEXT_COMPRESSION_NONE;
        titleText.key = mutableKey.data();
        titleText.text = mutableText.data();
        png_set_text(writeStruct, infoStruct, &titleText, 1);
    }*/

    png_write_info(writeStruct, infoStruct);

    //RGB 3-byte stride.
    const uint32_t rowStride = m_Width * 3;
    for (uint32_t row = 0; row < m_Height; row++)
    {
        uint32_t rowOffset = row * rowStride;
        png_write_row(writeStruct, &m_ImageData[rowOffset]);
    }

    png_write_end(writeStruct, infoStruct);

    png_free_data(writeStruct, infoStruct, PNG_FREE_ALL, -1);
    png_destroy_write_struct(&writeStruct, &infoStruct);
    return true;
}