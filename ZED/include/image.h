#pragma once
#include "core.h"
#include "blob.h"



namespace ZED
{
	class Image
	{
	public:
		Image() = default;
		Image(Image&& rhs) = default;
		DLLEXPORT Image(uint32_t Width, uint32_t Height, ColorType ColorType);
		DLLEXPORT Image(Blob&& Data, uint32_t Width, uint32_t Height, ColorType ColorType);

		virtual ~Image() {}

		uint32_t GetWidth() const { return m_Width; }
		uint32_t GetHeight() const { return m_Height; }
		ColorType GetColorType() const { return m_ColorType; }

		bool IsValid() const { return !m_ImageData.IsEmpty(); }
		operator bool () const { return IsValid(); }

		DLLEXPORT void Fill(Color Color);
		DLLEXPORT void CreateCircle();

		DLLEXPORT bool InsertAt(const Image& Source, uint32_t DestinationX, uint32_t DestinationY);

		DLLEXPORT Color GetPixel(uint32_t X, uint32_t Y);
		DLLEXPORT void  SetPixel(uint32_t X, uint32_t Y, Color NewColor);

		DLLEXPORT void ConvertTo(ColorType Format);
		DLLEXPORT void Flip();

		explicit operator Blob& () { return m_ImageData; }
		explicit operator const Blob& () const { return m_ImageData; }

		operator void* () { return m_ImageData; }
		operator const void* () const { return m_ImageData; }

		uint8_t& operator [] (uint32_t Index) { return m_ImageData[Index]; }
		const uint8_t& operator [] (uint32_t Index) const { return m_ImageData[Index]; }
		//uint8_t  operator [] (uint32_t Index) const { return m_ImageData[Index]; }

	protected:
		uint32_t m_Width  = 0;
		uint32_t m_Height = 0;

		ColorType m_ColorType = ColorType::R8G8B8;

		Blob m_ImageData;
	};
}