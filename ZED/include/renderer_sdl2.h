#pragma once
#include "renderer.h"
#include "blob.h"
#include "png.h"


struct SDL_Window;
struct SDL_Renderer;
struct SDL_Surface;
typedef struct _TTF_Font TTF_Font;



namespace ZED
{
	class RendererSDL2 : public Renderer
	{
	public:
		RendererSDL2() : Renderer() {}

		//Basics
		Type GetType() const override { return Type::SDL2; }
#ifdef _WIN32
		DLLEXPORT bool Init(HWND Hwnd) override;
#else
		DLLEXPORT bool Init(GtkWindow* DrawingArea) override;
#endif
		DLLEXPORT void ResetViewPort(uint32_t Width, uint32_t Height) override;
		DLLEXPORT void ClearDisplay() const override;
		DLLEXPORT void UpdateDisplay() const override;
		DLLEXPORT void Release() override;

		DLLEXPORT SDL_Renderer* GetSDL2Renderer() { return m_SDL_renderer; }

		//Fonts
		DLLEXPORT bool InitFonts(const char* FontFile, int Small = 7, int Middle = 10, int Large = 20, int Huge = 32, int Gigantic = -1, int Gigantic2 = -1) const override;
		DLLEXPORT virtual Ref<Texture> RenderFont(FontSize Size, const std::string& Text, ZED::Color Textcolor) const;

		DLLEXPORT Image TakeScreenshot() const override;
		DLLEXPORT void TakeScreenshotPNG(const char* Filename) const override;

		//Shader
		DLLEXPORT Ref<Shader> CompileShader(const char* VertexShaderSource, const char* PixelShaderSource) const override { return nullptr; }

		//Render calls
		DLLEXPORT void FillRect(const Rect& dstrect, unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255) const;

		DLLEXPORT void Render(const Texture& tex, int x, int y, bool render_hit = false) const {};
		DLLEXPORT void RenderPartial(const Texture& tex, int x, int y, int w, int h, bool render_hit = false) const {};
		DLLEXPORT void RenderRotated(const Texture& tex, int x, int y, bool render_hit = false) const {};

		DLLEXPORT void RenderTransformed(const Texture& tex, int x, int y) const override;
		DLLEXPORT void RenderTransformedPartial(const Texture& tex, int x, int y, int w, int h) const {};
		DLLEXPORT void RenderTransformedRotated(const Texture& tex, int x, int y) const;

	protected:
		SDL_Window *m_SDL_window = nullptr;

	private:
		SDL_Surface* m_SDL_screen = nullptr;

		SDL_Surface* m_SDL_window_screen = nullptr;

		SDL_Renderer* m_SDL_renderer = nullptr;

		mutable TTF_Font* m_ttf_font_small  = nullptr;
		mutable TTF_Font* m_ttf_font_medium = nullptr;
		mutable TTF_Font* m_ttf_font_large  = nullptr;
		mutable TTF_Font* m_ttf_font_huge   = nullptr;
		mutable TTF_Font* m_ttf_font_gigantic = nullptr;
		mutable TTF_Font* m_ttf_font_gigantic2 = nullptr;
	};



	class TextureSDL2 : public Texture
	{
	public:
		TextureSDL2(const std::string& Filename) : TextureSDL2(PNG(Filename)) {}
		DLLEXPORT TextureSDL2(int w, int h);
		DLLEXPORT TextureSDL2(SDL_Surface* Surface);
		DLLEXPORT TextureSDL2(const Image& Image);
		TextureSDL2() : Texture() {}
		~TextureSDL2() { TextureSDL2::Unload(); }

		TextureSDL2(Texture&& Original) noexcept
		{
			m_surface = Original.m_surface;
			Original.m_surface = nullptr;
		}

		TextureSDL2(TextureSDL2&& Original) noexcept
		{
			m_surface = Original.m_surface;
			Original.m_surface = nullptr;
		}

		void operator =(Texture&& Original) noexcept override
		{
			Unload();
			m_surface = Original.m_surface;
			Original.m_surface = nullptr;
		}

		void operator =(TextureSDL2&& Original) noexcept
		{
			Unload();
			m_surface = Original.m_surface;
			Original.m_surface = nullptr;
		}

		//Basics
		Type GetType() const override { return Type::SDL2; }
		DLLEXPORT void Unload() override;
		DLLEXPORT uint32_t GetWidth()  const override;//Returns the resolution-independent width of the texture. So, the texture width in SD quality. Returns 0 if surface is not loaded
		DLLEXPORT uint32_t GetHeight() const override;//Returns the resolution-independent height of the texture. So, the texture height in SD quality. Returns 0 if surface is not loaded

		DLLEXPORT void SetAngle(float angle_) override;

		//Texture manipulation
		DLLEXPORT void BindTexture(unsigned int Slot = 0) const override {}
		DLLEXPORT void CreateCircle() override;
		DLLEXPORT void CreateGradient(int r1, int g1, int b1, int r2, int g2, int b2);
		DLLEXPORT void CloneTo(TextureSDL2& Copy) const;
		DLLEXPORT void ConvertToGrayscale();
		DLLEXPORT void MultiplyWith(float Value);
		DLLEXPORT void Recolor(unsigned char r, unsigned char g, unsigned char b, float alpha = 0.9f);
		DLLEXPORT bool Compare(const Texture& Surface) const;
		DLLEXPORT void FlipSurface();
		DLLEXPORT void MirrorSurface();

		DLLEXPORT operator Blob () const;
		operator SDL_Surface* () { return m_surface; }

	protected:
		virtual bool IsTextureLoaded() const override { return true; }
		virtual bool LoadTexture() const override { return true; }

	private:
		DLLEXPORT void FillRectOnToMe(const Rect& dstrect, unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255);

		DLLEXPORT SDL_Surface* GetRotatedSurface() const override;


		mutable SDL_Surface* m_sdl_rotozoom = nullptr;
	};
}