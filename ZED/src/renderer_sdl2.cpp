#ifndef _WIN32
#include <gtk/gtk.h>
#endif
#include "../include/renderer_sdl2.h"
#include "../include/log.h"
#define SDL_DISABLE_IMMINTRIN_H
#include "../external/SDL2-2.0.14/include/SDL.h"
//#include "../external/SDL2-2.0.14/include/SDL_image.h"
#include "../external/SDL2-2.0.14/include/SDL_ttf.h"
#include "../external/SDL_rotozoom/SDL_rotozoom.h"
#ifndef _WIN32
#include <gdk/gdkx.h>//Include last because of macros
#endif



#ifdef _DEBUG
//#pragma comment(lib, "../external/SDL2_ttf-2.0.15/VisualC/Win32/Debug/SDL2_ttf.lib")
#else
//#pragma comment(lib, "../external/SDL2_ttf-2.0.15/VisualC/Win32/Release/SDL2_ttf.lib")
#endif


using namespace ZED;



#ifdef _WIN32
bool RendererSDL2::Init(HWND Hwnd)
#else
bool RendererSDL2::Init(GtkWindow* DrawingArea)
#endif
{
	Log::Info("Starting SDL2 renderer");

	//Make sure the window has been created and has received WM_CREATE otherwise SDL2 might crash in SetProp() in SDL_CreateWindowFrom()
#ifdef _WIN32
	if (!IsWindow(Hwnd))
	{
		Log::Error("Invalid window handle");
		return false;
	}
#endif


	//Start SDL
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER);

	//const int flags = IMG_INIT_PNG;

	//if ((IMG_Init(flags) & flags) != flags)
		//return false;

#ifdef _WIN32
	SDL_Window* pOpenGLDummyWin;

	{//Only needed for OpenGL
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 1);
		pOpenGLDummyWin = SDL_CreateWindow("", 0, 0, 1, 1, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);

		char sBuf[32];
		sprintf_s<32>(sBuf, "%p", pOpenGLDummyWin);

		SDL_SetHint(SDL_HINT_VIDEO_WINDOW_SHARE_PIXEL_FORMAT, sBuf);
	}

	//m_SDL_window = SDL_CreateWindowFrom(Hwnd, SDL_TRUE);
	m_SDL_window = SDL_CreateWindowFrom(Hwnd, SDL_TRUE);

	if (!m_SDL_window)
	{
		Log::Error("Could not create SDL window");
		return false;
	}

	RECT rect;
	GetClientRect(Hwnd, &rect);
	m_screen_w = rect.right;
	m_screen_h = rect.bottom;
#else
	if (DrawingArea)
	{
		void* gdk_window = gtk_widget_get_window(GTK_WIDGET(DrawingArea));//Get GDK window of our drawing area
		void* window_id = (void*)(intptr_t)GDK_WINDOW_XID(gdk_window);//Get X11 id of the GDK window

		m_SDL_window = SDL_CreateWindowFrom(window_id, SDL_TRUE);

		gtk_window_get_size(GTK_WINDOW(DrawingArea), &m_screen_w, &m_screen_h);
	}
	else
	{
		m_screen_w = 1920;
		m_screen_h = 1080;
	}
#endif

	m_SDL_window_screen = SDL_GetWindowSurface(m_SDL_window);

	Uint32 rmask = 0x00ff0000;
	Uint32 gmask = 0x0000ff00;
	Uint32 bmask = 0x000000ff;
	Uint32 amask = 0x00000000;
	m_SDL_screen = SDL_CreateRGBSurface(0, m_screen_w, m_screen_h, 24, rmask, gmask, bmask, amask);

	//m_SDL_renderer = SDL_CreateRenderer(m_SDL_window, -1, SDL_RENDERER_ACCELERATED);

	ZED::Log::Debug("SDL2 renderer initialized with size " + std::to_string(m_screen_w) + "x" + std::to_string(m_screen_h));
	return true;
}



void RendererSDL2::ResetViewPort(uint32_t Width, uint32_t Height)
{
	m_screen_w = Width;
	m_screen_h = Height;

	m_SDL_window_screen = SDL_GetWindowSurface(m_SDL_window);

	SDL_FreeSurface(m_SDL_screen);

	Uint32 rmask = 0x00ff0000;
	Uint32 gmask = 0x0000ff00;
	Uint32 bmask = 0x000000ff;
	Uint32 amask = 0x00000000;
	m_SDL_screen = SDL_CreateRGBSurface(0, m_screen_w, m_screen_h, 24, rmask, gmask, bmask, amask);
}



void RendererSDL2::ClearDisplay() const
{
}



void RendererSDL2::UpdateDisplay() const
{
	SDL_BlitScaled(m_SDL_screen, NULL, m_SDL_window_screen, NULL);
	SDL_UpdateWindowSurface(m_SDL_window);
}



void RendererSDL2::Release()
{
	if (m_SDL_screen)
	{
		SDL_FreeSurface(m_SDL_screen);
		m_SDL_screen = nullptr;
	}

	if (m_SDL_window)
	{
		SDL_DestroyWindow(m_SDL_window);
		m_SDL_window = nullptr;
		SDL_Quit();
	}
}



bool RendererSDL2::InitFonts(const char* FontFile, int Small, int Middle, int Large, int Huge, int Gigantic, int Gigantic2) const
{
	Log::Debug("Initializing fonts");
	if (!TTF_WasInit())
	{
		if (TTF_Init() != 0)
		{
			Log::Error("SDL TTF could not be initialized");
			return false;
		}
	}

	m_ttf_font_small  = TTF_OpenFont(FontFile, Small);
	m_ttf_font_medium = TTF_OpenFont(FontFile, Middle);
	m_ttf_font_large  = TTF_OpenFont(FontFile, Large);
	m_ttf_font_huge   = TTF_OpenFont(FontFile, Huge);
	if (Gigantic > 0)
		m_ttf_font_gigantic = TTF_OpenFont(FontFile, Gigantic);
	if (Gigantic2 > 0)
		m_ttf_font_gigantic2 = TTF_OpenFont(FontFile, Gigantic2);

	if (!m_ttf_font_small || !m_ttf_font_medium || !m_ttf_font_large || !m_ttf_font_huge)
		Log::Warn("Font file could not be opened");
	else
		Log::Info("Fonts initialized");

	return true;
}



Ref<Texture> RendererSDL2::RenderFont(FontSize Size, const std::string& Text, ZED::Color Textcolor) const
{
	SDL_Color color;
	color.a = 255;
	color.r = Textcolor.r;
	color.g = Textcolor.g;
	color.b = Textcolor.b;

	TextureSDL2* ret = nullptr;

	switch (Size)
	{
		case FontSize::Small:
			if (m_ttf_font_small)
				ret = new TextureSDL2(TTF_RenderUTF8_Solid(m_ttf_font_small, Text.c_str(), color));
				//ret = new TextureSDL2(TTF_RenderUTF8_Blended(m_ttf_font_small, Text.c_str(), color));
			break;
		case FontSize::Middle:
			if (m_ttf_font_medium)
				ret = new TextureSDL2(TTF_RenderUTF8_Solid(m_ttf_font_medium, Text.c_str(), color));
				//ret = new TextureSDL2(TTF_RenderUTF8_Blended(m_ttf_font_medium, Text.c_str(), color));
			break;
		case FontSize::Large:
			if (m_ttf_font_large)
				ret = new TextureSDL2(TTF_RenderUTF8_Solid(m_ttf_font_large, Text.c_str(), color));
				//ret = new TextureSDL2(TTF_RenderUTF8_Blended(m_ttf_font_large, Text.c_str(), color));
			break;
		case FontSize::Huge:
			if (m_ttf_font_huge)
				ret = new TextureSDL2(TTF_RenderUTF8_Solid(m_ttf_font_huge, Text.c_str(), color));
				//ret = new TextureSDL2(TTF_RenderUTF8_Blended(m_ttf_font_huge, Text.c_str(), color));
			break;
		case FontSize::Gigantic:
			if (m_ttf_font_gigantic)
				ret = new TextureSDL2(TTF_RenderUTF8_Solid(m_ttf_font_gigantic, Text.c_str(), color));
				//ret = new TextureSDL2(TTF_RenderUTF8_Blended(m_ttf_font_gigantic, Text.c_str(), color));
			break;
		case FontSize::Gigantic2:
			if (m_ttf_font_gigantic2)
				ret = new TextureSDL2(TTF_RenderUTF8_Solid(m_ttf_font_gigantic2, Text.c_str(), color));
				//ret = new TextureSDL2(TTF_RenderUTF8_Blended(m_ttf_font_gigantic2, Text.c_str(), color));
			break;
	}

	if (ret)
	{
		//Log::Debug("Text rendered to texture, width=" + std::to_string(ret->GetWidth()));
		strcpy_s(ret->m_image_filename, sizeof(ret->m_image_filename), Text.c_str());
	}
	else
		Log::Warn("Failed to create texture for text");

	return ret;
}



Image RendererSDL2::TakeScreenshot() const
{
	Blob data(m_SDL_screen->pixels, m_screen_w * m_screen_h * 3);
	Image img(std::move(data), m_screen_w, m_screen_h, ColorType::B8G8R8);
	return img;
	//img.ConvertTo(ColorType::R8G8B8);
	//return Image(PNG::Compress(img), m_screen_w, m_screen_h, ColorType::R8G8B8);
	//PNG Image(std::move(img));
	//PNG Image(std::move(data), m_screen_w, m_screen_h, ColorType::B8G8R8);
	//Image.ConvertTo(ColorType::R8G8B8);
	//return Image;
}



void RendererSDL2::TakeScreenshotPNG(const char* Filename) const
{
	Blob data(m_SDL_screen->pixels, m_screen_w * m_screen_h * 3);

	PNG Image(std::move(data), m_screen_w, m_screen_h, ColorType::B8G8R8);
	Image.ConvertTo(ColorType::R8G8B8);
	//Image.Flip();
	Image.Save(Filename);
}



void RendererSDL2::FillRect(const Rect& dstrect, unsigned char r, unsigned char g, unsigned char b, unsigned char a) const
{
	if (!m_SDL_screen)
		return;

	SDL_Rect rect;
	rect.x = dstrect.x;
	rect.y = dstrect.y;
	rect.w = dstrect.w;
	rect.h = dstrect.h;

	SDL_FillRect(m_SDL_screen, &rect, SDL_MapRGB(m_SDL_screen->format, r, g, b));
}



void ZED::RendererSDL2::RenderTransformed(const Texture& tex, int x, int y) const
{
	if (!tex.m_surface)
		return;

	m_DrawCalls++;

	SDL_Rect dest;
	dest.x = x;
	dest.y = y;
	dest.w = tex.m_surface->w;
	dest.h = tex.m_surface->h;
	SDL_BlitSurface(tex.m_surface, NULL, m_SDL_screen, &dest);
}



void ZED::RendererSDL2::RenderTransformedRotated(const Texture& tex, int x, int y) const
{
	if (!tex.m_surface)
		return;

	m_DrawCalls++;

	SDL_Surface* render_surface = tex.m_surface;

	//should we be using the rotozoom surface?
	if (!ZED::isz(tex.m_angle) || !ZED::is1(tex.m_sizeX) || !ZED::is1(tex.m_sizeY))
		render_surface = tex.GetRotatedSurface();

	/*if (about_center)
	{
		x -= render_surface->w >> 1;
		y -= render_surface->h >> 1;
	}*/

	SDL_Rect dest;
	dest.x = x;
	dest.y = y;
	dest.w = tex.m_surface->w;
	dest.h = tex.m_surface->h;

	/*if (render_hit)
		BlitHitSurface(&from_rect, &to_rect, NULL, true);
	else*/
		SDL_BlitSurface(render_surface, NULL, m_SDL_screen, &dest);
}



TextureSDL2::TextureSDL2(int w, int h) : Texture()
{
	//	new_surface = SDL_CreateRGBSurface(NULL, w, h, 32, 0xFF000000, 0x0000FF00, 0x00FF0000, 0x000000FF);//SDL 1.2 -> 2.0
	Uint32 rmask = 0x000000ff;
	Uint32 gmask = 0x0000ff00;
	Uint32 bmask = 0x00ff0000;
	Uint32 amask = 0x00000000;
	m_surface = SDL_CreateRGBSurface(NULL, w, h, 24, rmask, gmask, bmask, amask);//SDL 1.2 -> 2.0

	FillRectOnToMe(Rect(0, 0, w, h), 0, 0, 0);
}



TextureSDL2::TextureSDL2(SDL_Surface* Surface) : Texture()
{
	m_surface = Surface;

	if (!m_surface)
	{
		Log::Error("Could not copy Surface");
		return;
	}

	//convert to a guaranteed good format
	SDL_Surface* new_ret;

#ifdef SDL1
	new_ret = SDL_DisplayFormatAlpha(m_surface);//SDL 1.2 -> 2.0
#else
	//new_ret = SDL_ConvertSurfaceFormat(m_surface, SDL_PIXELFORMAT_ARGB8888, NULL);
	//new_ret = SDL_ConvertSurfaceFormat(m_surface, SDL_PIXELFORMAT_RGBA8888, NULL);
	new_ret = SDL_ConvertSurfaceFormat(m_surface, SDL_PIXELFORMAT_RGBA32, NULL);
#endif

	SDL_FreeSurface(m_surface);

	m_surface = new_ret;

	//SDL_SetSurfaceBlendMode(m_surface, SDL_BLENDMODE_BLEND);

	/*if (!delete_surface)
		m_sdl_texture = SDL_CreateTextureFromSurface(g_SDL_renderer, m_surface);*/
}



TextureSDL2::TextureSDL2(const Image& Image)
{
	if (!Image)
	{
		Log::Error("Invalid image data");
		return;
	}

	Uint32 rmask = 0x000000ff;
	Uint32 gmask = 0x0000ff00;
	Uint32 bmask = 0x00ff0000;

	int bpp = ColorTypeSize(Image.GetColorType());

	if (bpp == 4)
		m_surface = SDL_CreateRGBSurface(NULL, Image.GetWidth(), Image.GetHeight(), 32, rmask, gmask, bmask, 0xff000000);
	else
		m_surface = SDL_CreateRGBSurface(NULL, Image.GetWidth(), Image.GetHeight(), 24, rmask, gmask, bmask, 0x00000000);

	if (!m_surface)
	{
		Log::Error("Could not create SDL2 surface for texture");
		return;
	}

	for (int x = 0; x < m_surface->w; x++)
	{
		for (int y = 0; y < m_surface->h; y++)
		{
			Uint8* pixel = ((Uint8*)m_surface->pixels + y * m_surface->pitch + x * m_surface->format->BytesPerPixel);
			const int index = (y * Image.GetWidth() + x) * bpp;

			pixel[0] = Image[index + 0];
			pixel[1] = Image[index + 1];
			pixel[2] = Image[index + 2];
			if (m_surface->format->BytesPerPixel == 4)
				pixel[3] = Image[index + 3];
		}
	}
}



uint32_t TextureSDL2::GetWidth() const
{
	if (!m_surface)
		return 0;
	return m_surface->w / (int)m_quality;
}



uint32_t TextureSDL2::GetHeight() const
{
	if (!m_surface)
		return 0;
	return m_surface->h / (int)m_quality;
}



void TextureSDL2::SetAngle(float angle_)
{
	m_angle = angle_;
	if (m_sdl_rotozoom)
	{
		SDL_FreeSurface(m_sdl_rotozoom);
		m_sdl_rotozoom = nullptr;
	}
}



void TextureSDL2::ConvertToGrayscale()
{
	if (!m_surface) return;

	for (int x = 0; x < m_surface->w; x++)
	{
		for (int y = 0; y < m_surface->h; y++)
		{
			Uint32* pixel = (Uint32*)((Uint8*)m_surface->pixels + y * m_surface->pitch + x * m_surface->format->BytesPerPixel);

			float r = ((Uint8*)pixel)[2];
			float g = ((Uint8*)pixel)[1];
			float b = ((Uint8*)pixel)[0];

			const unsigned char out = (unsigned char)(0.299f * r + 0.587f * g + 0.114f * b);

			((Uint8*)pixel)[0] = (Uint8)out;
			((Uint8*)pixel)[1] = (Uint8)out;
			((Uint8*)pixel)[2] = (Uint8)out;
		}
	}
}



void TextureSDL2::MultiplyWith(float Value)
{
	if (!m_surface) return;

	for (int x = 0; x < m_surface->w; x++)
	{
		for (int y = 0; y < m_surface->h; y++)
		{
			Uint32* pixel = (Uint32*)((Uint8*)m_surface->pixels + y * m_surface->pitch + x * m_surface->format->BytesPerPixel);

			float r = ((Uint8*)pixel)[2];
			float g = ((Uint8*)pixel)[1];
			float b = ((Uint8*)pixel)[0];

			((Uint8*)pixel)[0] = (Uint8)(r * Value);
			((Uint8*)pixel)[1] = (Uint8)(g * Value);
			((Uint8*)pixel)[2] = (Uint8)(b * Value);
		}
	}
}



void TextureSDL2::Recolor(unsigned char r, unsigned char g, unsigned char b, float alpha)
{
	if (!m_surface) return;

	const float nr = (float)r;
	const float ng = (float)g;
	const float nb = (float)b;

	for (int x = 0; x < m_surface->w; x++)
	{
		for (int y = 0; y < m_surface->h; y++)
		{
			Uint32* pixel = (Uint32*)((Uint8*)m_surface->pixels + y * m_surface->pitch + x * m_surface->format->BytesPerPixel);

			Uint8 r = ((Uint8*)pixel)[2];
			Uint8 g = ((Uint8*)pixel)[1];
			Uint8 b = ((Uint8*)pixel)[0];

			if (m_surface->format->BytesPerPixel == 4)
			{
				r = ((Uint8*)pixel)[0];
				//g = ((Uint8*)pixel)[1];
				b = ((Uint8*)pixel)[2];
			}

			r = (Uint8)((1.0f - alpha) * r + alpha * nr);
			g = (Uint8)((1.0f - alpha) * g + alpha * ng);
			b = (Uint8)((1.0f - alpha) * b + alpha * nb);

			if (m_surface->format->BytesPerPixel == 3)
			{
				((Uint8*)pixel)[2] = (Uint8)r;
				((Uint8*)pixel)[1] = (Uint8)g;
				((Uint8*)pixel)[0] = (Uint8)b;
			}

			else if (m_surface->format->BytesPerPixel == 4)
			{
				((Uint8*)pixel)[0] = (Uint8)r;
				((Uint8*)pixel)[1] = (Uint8)g;
				((Uint8*)pixel)[2] = (Uint8)b;
			}
		}
	}
}



bool TextureSDL2::Compare(const Texture& Surface) const
{
	if (!m_surface) return false;
	if (!Surface.GetSurface()) return false;

	if (m_surface->w != Surface.m_surface->w) return false;
	if (m_surface->h != Surface.m_surface->h) return false;

	for (int x = 0; x < m_surface->w; x++)
	{
		for (int y = 0; y < m_surface->h; y++)
		{
			Uint8* pixel  = ((Uint8*)m_surface->pixels + y * m_surface->pitch + x * m_surface->format->BytesPerPixel);
			Uint8* pixel2 = ((Uint8*)Surface.m_surface->pixels + y * Surface.m_surface->pitch + x * Surface.m_surface->format->BytesPerPixel);

			Uint8 r = pixel[0];
			Uint8 g = pixel[1];
			Uint8 b = pixel[2];
			Uint8 a = pixel[3];

			Uint8 r2 = pixel2[0];
			Uint8 g2 = pixel2[1];
			Uint8 b2 = pixel2[2];
			Uint8 a2 = pixel2[3];

			if (r != r2)
				return false;
			if (g != g2)
				return false;
			if (b != b2)
				return false;
			if (a != a2)
				return false;
		}
	}

	return true;
}



void TextureSDL2::FlipSurface()
{
	if (!m_surface) return;

	//Flip image
	unsigned char* pixels = (unsigned char*)m_surface->pixels;
	for (int y = 0; y < m_surface->h/2; y++)
	{
		for (int x = 0; x < m_surface->w; x++)
		{
			unsigned char temp1 = pixels[y * m_surface->pitch + x * 3 + 0];
			unsigned char temp2 = pixels[y * m_surface->pitch + x * 3 + 1];
			unsigned char temp3 = pixels[y * m_surface->pitch + x * 3 + 2];

			pixels[y * m_surface->pitch + x * 3 + 0] = pixels[(m_surface->h - 1 - y) * m_surface->pitch + x * 3 + 0];
			pixels[y * m_surface->pitch + x * 3 + 1] = pixels[(m_surface->h - 1 - y) * m_surface->pitch + x * 3 + 1];
			pixels[y * m_surface->pitch + x * 3 + 2] = pixels[(m_surface->h - 1 - y) * m_surface->pitch + x * 3 + 2];

			pixels[(m_surface->h - 1 - y) * m_surface->pitch + x * 3 + 0] = temp1;
			pixels[(m_surface->h - 1 - y) * m_surface->pitch + x * 3 + 1] = temp2;
			pixels[(m_surface->h - 1 - y) * m_surface->pitch + x * 3 + 2] = temp3;
		}
	}
}



void TextureSDL2::MirrorSurface()
{
	if (!m_surface) return;

	//Mirror image
	unsigned char* pixels = (unsigned char*)m_surface->pixels;
	for (int y = 0; y < m_surface->h; y++)
	{
		for (int x = 0; x < m_surface->w / 2; x++)
		{
			unsigned char temp1 = pixels[y * m_surface->pitch + x * 3 + 0];
			unsigned char temp2 = pixels[y * m_surface->pitch + x * 3 + 1];
			unsigned char temp3 = pixels[y * m_surface->pitch + x * 3 + 2];

			pixels[y * m_surface->pitch + x * 3 + 0] = pixels[y * m_surface->pitch + (m_surface->w - 1 - x) * 3 + 0];
			pixels[y * m_surface->pitch + x * 3 + 1] = pixels[y * m_surface->pitch + (m_surface->w - 1 - x) * 3 + 1];
			pixels[y * m_surface->pitch + x * 3 + 2] = pixels[y * m_surface->pitch + (m_surface->w - 1 - x) * 3 + 2];

			pixels[y * m_surface->pitch + (m_surface->w - 1 - x) * 3 + 0] = temp1;
			pixels[y * m_surface->pitch + (m_surface->w - 1 - x) * 3 + 1] = temp2;
			pixels[y * m_surface->pitch + (m_surface->w - 1 - x) * 3 + 2] = temp3;
		}
	}
}



void TextureSDL2::CreateCircle()
{
	if (!m_surface) return;

	float center_x = m_surface->w / 2.0f;
	float center_y = m_surface->h / 2.0f;

	const float radius = m_surface->w - center_x;

	for (int x = 0; x < m_surface->w; x++)
	{
		for (int y = 0; y < m_surface->h; y++)
		{
			Uint8* pixel = ((Uint8*)m_surface->pixels + y * m_surface->pitch + x * m_surface->format->BytesPerPixel);

			const float dx = x - center_x;
			const float dy = y - center_y;

			if (dx * dx + dy * dy <= radius * radius)
			{
				pixel[0] = 255;
				pixel[1] = 255;
				pixel[2] = 255;
				if (m_surface->format->BytesPerPixel == 4)
					pixel[3] = 255;
			}
			else
			{
				pixel[0] = 0;
				pixel[1] = 0;
				pixel[2] = 0;
				if (m_surface->format->BytesPerPixel == 4)
					pixel[3] = 0;
			}
		}
	}
}



void TextureSDL2::CreateGradient(int r1, int g1, int b1, int r2, int g2, int b2)
{
	if (!m_surface) return;

	for (int x = 0; x < m_surface->w; x++)
	{
		for (int y = 0; y < m_surface->h; y++)
		{
			Uint8* pixel = ((Uint8*)m_surface->pixels + y * m_surface->pitch + x * m_surface->format->BytesPerPixel);

			uint8_t r = (uint8_t) ( (1.0 - (double)x / (double)m_surface->w) * r1 + (double)x / (double)m_surface->w * r2 );
			uint8_t g = (uint8_t) ( (1.0 - (double)x / (double)m_surface->w) * g1 + (double)x / (double)m_surface->w * g2 );
			uint8_t b = (uint8_t) ( (1.0 - (double)x / (double)m_surface->w) * b1 + (double)x / (double)m_surface->w * b2 );

			pixel[0] = r;
			pixel[1] = g;
			pixel[2] = b;
			if (m_surface->format->BytesPerPixel == 4)
				pixel[3] = 255;
		}
	}
}



void TextureSDL2::CloneTo(TextureSDL2& Copy) const
{
	if (!m_surface) return;

	if (Copy.m_surface)
		Copy.Unload();

	Copy.m_surface = SDL_CreateRGBSurface(NULL, m_surface->w, m_surface->h, m_surface->format->BitsPerPixel,
										  m_surface->format->Rmask, m_surface->format->Gmask, m_surface->format->Bmask, m_surface->format->Amask);

	SDL_BlitSurface(m_surface, NULL, Copy.m_surface, NULL);
}



TextureSDL2::operator Blob () const
{
	if (!m_surface) return Blob();

	Blob ret(m_surface->w * m_surface->h * m_surface->format->BytesPerPixel);

	for (int y = 0; y < m_surface->h; y++)
	{
		Uint32* pixels = (Uint32*)((Uint8*)m_surface->pixels + y * m_surface->pitch);

		//Copy row
		ret.Append(pixels, m_surface->w * m_surface->format->BytesPerPixel);
	}

	return ret;
}



void TextureSDL2::Unload()
{
	if (m_surface)
	{
		SDL_FreeSurface(m_surface);
		m_surface = nullptr;
	}
	if (m_sdl_rotozoom)
	{
		SDL_FreeSurface(m_sdl_rotozoom);
		m_sdl_rotozoom = nullptr;
	}
}



void TextureSDL2::FillRectOnToMe(const Rect& dstrect, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	if (!m_surface) return;

	if (!m_surface->format) return;//If 8bit image, format is NULL

	SDL_FillRect(m_surface, (SDL_Rect*)&dstrect, SDL_MapRGBA(m_surface->format, r, g, b, a));

	//unload rotozoom surface?
	if (m_sdl_rotozoom)
		SDL_FreeSurface(m_sdl_rotozoom);
	m_sdl_rotozoom = nullptr;
}



SDL_Surface* TextureSDL2::GetRotatedSurface() const
{
	if (!m_sdl_rotozoom)
	{
		if (!m_surface)
		{
			Log::Warn("Surface not loaded");
			return nullptr;
		}

		m_sdl_rotozoom = rotozoomSurface(m_surface, m_angle, m_sizeX, 0);

		SDL_SetSurfaceBlendMode(m_sdl_rotozoom, SDL_BLENDMODE_BLEND);

		//if (!m_sdl_rotozoom)
			//OutputDebugStringA("LoadRotoZoomSurface::sdl_rotozoom for %s not created (angle:%f size:%f)\n");
	}

	return m_sdl_rotozoom;
}