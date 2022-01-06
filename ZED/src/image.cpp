#include "../include/image.h"
#include "../include/log.h"


using namespace ZED;



Image::Image(uint32_t Width, uint32_t Height, ColorType ColorType) :
	m_Width(Width), m_Height(Height), m_ColorType(ColorType),
	m_ImageData(Width*Height* ColorTypeSize(ColorType))
{
}



Image::Image(Blob&& Data, uint32_t Width, uint32_t Height, ColorType ColorType) :
	m_Width(Width), m_Height(Height), m_ColorType(ColorType),
	m_ImageData(std::move(Data))
{
}



void Image::Fill(Color Color)
{
	int bpp = ColorTypeSize(m_ColorType);

	for (uint32_t x = 0; x < m_Width; x++)
	{
		for (uint32_t y = 0; y < m_Height; y++)
		{
			uint8_t* pixel = &m_ImageData[(y * m_Width + x) * bpp];

			pixel[0] = Color.r;
			pixel[1] = Color.g;
			pixel[2] = Color.b;
			if (bpp == 4)
				pixel[3] = Color.a;
		}
	}
}



void Image::CreateCircle()
{
	float center_x = m_Width  / 2.0f;
	float center_y = m_Height / 2.0f;

	const float radius = m_Width - center_x;

	int bpp = ColorTypeSize(m_ColorType);

	for (uint32_t x = 0; x < m_Width; x++)
	{
		for (uint32_t y = 0; y < m_Height; y++)
		{
			uint8_t* pixel = &m_ImageData[(y * m_Width + x) * bpp];

			const float dx = x - center_x;
			const float dy = y - center_y;

			if (dx * dx + dy * dy <= radius * radius)
			{
				pixel[0] = 255;
				pixel[1] = 255;
				pixel[2] = 255;
				if (bpp == 4)
					pixel[3] = 255;
			}
			else
			{
				pixel[0] = 0;
				pixel[1] = 0;
				pixel[2] = 0;
				if (bpp == 4)
					pixel[3] = 0;
			}
		}
	}
}



bool Image::InsertAt(const Image& Source, uint32_t DestinationX, uint32_t DestinationY)
{
	if (m_ImageData.IsEmpty()) return false;
	if (m_Width  < Source.GetWidth()  + DestinationX) return false;
	if (m_Height < Source.GetHeight() + DestinationY) return false;

	int dest_bpp   = ColorTypeSize(m_ColorType);
	int source_bpp = ColorTypeSize(Source.GetColorType());

	if (source_bpp > dest_bpp) return false;

	for (uint32_t x = 0; x < Source.GetWidth(); x++)
	{
		for (uint32_t y = 0; y < Source.GetHeight(); y++)
		{
			for (int i=0;i < dest_bpp;i++)
				m_ImageData[((y + DestinationY) * m_Width + x + DestinationX) * dest_bpp + i] = Source.m_ImageData[(y * Source.GetWidth() + x) * source_bpp + i];
			for (int i = dest_bpp; i < source_bpp; i++)
				m_ImageData[((y + DestinationY) * m_Width + x + DestinationX) * dest_bpp + i] = 255;
		}
	}

	return true;
}



Color Image::GetPixel(uint32_t X, uint32_t Y)
{
	if (m_ImageData.IsEmpty())
		return Color();

	const unsigned int bpp = ColorTypeSize(m_ColorType);
	const unsigned int pitch = m_Width * bpp;
	const unsigned int index = Y * pitch + X * bpp;

	Color ret(0, 0, 0, 255);

	switch (m_ColorType)
	{
	case ColorType::R8G8B8A8:
		ret.a = m_ImageData[index + 3];
	case ColorType::R8G8B8://Fallthrough
		ret.r = m_ImageData[index + 0];
		ret.g = m_ImageData[index + 1];
		ret.b = m_ImageData[index + 2];
		break;

	case ColorType::B8G8R8:
		ret.b = m_ImageData[index + 0];
		ret.g = m_ImageData[index + 1];
		ret.r = m_ImageData[index + 2];
		break;
	}

	return ret;
}



void Image::SetPixel(uint32_t X, uint32_t Y, Color NewColor)
{
	const unsigned int bpp = ColorTypeSize(m_ColorType);
	const unsigned int pitch = m_Width * bpp;
	const unsigned int index = Y * pitch + X * bpp;

	switch (m_ColorType)
	{
	case ColorType::R8G8B8A8:
		m_ImageData[index + 3] = NewColor.a;
	case ColorType::R8G8B8:
		m_ImageData[index + 0] = NewColor.r;
		m_ImageData[index + 1] = NewColor.g;
		m_ImageData[index + 2] = NewColor.b;
		break;

	case ColorType::B8G8R8:
		m_ImageData[index + 0] = NewColor.b;
		m_ImageData[index + 1] = NewColor.g;
		m_ImageData[index + 2] = NewColor.r;
		break;
	}
}



void Image::ConvertTo(ColorType Format)
{
	if (ColorTypeSize(m_ColorType) < ColorTypeSize(Format))
	{
		Log::Error("Can not convert to formats of different size");
		return;
	}

	//unsigned int bpp = ColorTypeSize(m_ColorType);
	//unsigned int pitch = m_Width * bpp;

	for (unsigned int y = 0; y < m_Height; y++)
	{
		for (unsigned int x = 0; x < m_Width; x++)
		{
			auto temp = m_ColorType;
			auto pixel = GetPixel(x, y);

			m_ColorType = Format;
			SetPixel(x, y, pixel);
			m_ColorType = temp;
		}
	}

	m_ColorType = Format;
}



void Image::Flip()
{
	unsigned int bpp = ColorTypeSize(m_ColorType);
	unsigned int pitch = m_Width * bpp;

	if (bpp != 3)
	{
		Log::Error("Image::Flip() Not implemented");
		return;
	}

	for (unsigned int y = 0; y < m_Height/2; y++)
	{
		for (unsigned int x = 0; x < m_Width; x++)
		{
			unsigned char temp1 = m_ImageData[y * pitch + x * bpp + 0];
			unsigned char temp2 = m_ImageData[y * pitch + x * bpp + 1];
			unsigned char temp3 = m_ImageData[y * pitch + x * bpp + 2];

			m_ImageData[y * pitch + x * bpp + 0] = m_ImageData[(m_Height - 1 - y) * pitch + x * bpp + 0];
			m_ImageData[y * pitch + x * bpp + 1] = m_ImageData[(m_Height - 1 - y) * pitch + x * bpp + 1];
			m_ImageData[y * pitch + x * bpp + 2] = m_ImageData[(m_Height - 1 - y) * pitch + x * bpp + 2];

			m_ImageData[(m_Height - 1 - y) * pitch + x * bpp + 0] = temp1;
			m_ImageData[(m_Height - 1 - y) * pitch + x * bpp + 1] = temp2;
			m_ImageData[(m_Height - 1 - y) * pitch + x * bpp + 2] = temp3;
		}
	}
}