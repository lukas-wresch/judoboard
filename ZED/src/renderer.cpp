#include <fstream>
#include "../include/renderer.h"
#include "../include/renderer_sdl2.h"
#ifndef NO_OPENGL
#include "../include/renderer_opengl.h"
#endif
#include "../include/renderer_opengles1_1.h"
#include "../include/png.h"
#include "../include/log.h"
#define SDL_DISABLE_IMMINTRIN_H
#include "../external/SDL2-2.0.14/include/SDL.h"
//#include "../external/SDL2-2.0.14/include/SDL_image.h"


#ifdef _DEBUG
#pragma comment(lib, "../external/SDL2-2.0.14/VisualC/Win32/Debug/SDL2.lib")
#pragma comment(lib, "../external/SDL2_image-2.0.5/VisualC/Win32/Debug/SDL2_image.lib")
#pragma comment(lib, "../external/SDL2_ttf-2.0.15/VisualC/Win32/Debug/SDL2_ttf.lib")
//#pragma comment(lib, "../external/SDL2_ttf-2.0.15/VisualC/external/lib/x86/libfreetype-6.lib")
#pragma comment(lib, "../external/freetype-2.10.4/objs/Win32/Debug Static/freetype.lib")
#else
#pragma comment(lib, "../external/SDL2-2.0.14/VisualC/Win32/Release/SDL2.lib")
#pragma comment(lib, "../external/SDL2_image-2.0.5/VisualC/Win32/Release/SDL2_image.lib")
#pragma comment(lib, "../external/SDL2_ttf-2.0.15/VisualC/Win32/Release/SDL2_ttf.lib")
//#pragma comment(lib, "../external/SDL2_ttf-2.0.15/VisualC/external/lib/x86/libfreetype-6.lib")
#pragma comment(lib, "../external/freetype-2.10.4/objs/Win32/Release Static/freetype.lib")
#endif


using namespace ZED;



Ref<Texture> Renderer::CreateTexture() const
{
	if (GetType() == Type::SDL2)
		return new TextureSDL2;
#ifndef NO_OPENGL
	else if (GetType() == Type::OpenGL)
		return new TextureOpenGL;
#endif
	else if (GetType() == Type::OpenGLES1_1)
		return new TextureOpenGLES1_1;
	return nullptr;
}



Ref<Texture>Renderer::CreateTexture(int w, int h) const
{
	if (GetType() == Type::SDL2)
		return new TextureSDL2(w, h);
#ifndef NO_OPENGL
	else if (GetType() == Type::OpenGL)
		return new TextureOpenGL(w, h);
#endif
	else if (GetType() == Type::OpenGLES1_1)
		return new TextureOpenGLES1_1(w, h);
	return nullptr;
}



Ref<Texture> Renderer::CreateTexture(const char* Filename) const
{
	if (GetType() == Type::SDL2)
		return new TextureSDL2(Filename);
#ifndef NO_OPENGL
	else if (GetType() == Type::OpenGL)
		return new TextureOpenGL(Filename);
#endif
	//else if (GetType() == Type::OpenGLES1_1)
		//return new TextureOpenGLES1_1(Filename);
	return nullptr;
}



Ref<Texture> Renderer::CreateTexture(const Image& Image) const
{
	if (GetType() == Type::SDL2)
		return new TextureSDL2(Image);
#ifndef NO_OPENGL
	else if (GetType() == Type::OpenGL)
		return new TextureOpenGL(Image);
#endif
	else if (GetType() == Type::OpenGLES1_1)
		return new TextureOpenGLES1_1(Image);
	return nullptr;
}



Ref<Shader> Renderer::CompileShader(const std::string& Filename) const
{
	std::ifstream file(Filename);

	if (!file)
		return nullptr;

	file.seekg(0, file.end);
	size_t length = (size_t)file.tellg();
	file.seekg(0, file.beg);

	char* source = new char[length + 1];
	file.read(source, length);
	source[length] = '\0';

	auto ret = CompileShader(source, source);
	delete[] source;
	return ret;
}



void Renderer::GetScroll(int& X, int& Y) const
{
	X = m_shift_x;
	Y = m_shift_y;
}



void Renderer::SetScroll(int X, int Y)
{
	m_shift_x = X;
	m_shift_y = Y;
}



void Renderer::Scroll(int X, int Y)
{
	m_shift_x += X;
	m_shift_y += Y;

	if (m_shift_x < 0)
		m_shift_x = 0;
	if (m_shift_y < 0)
		m_shift_y = 0;
}



void Renderer::WorldToScreen(int WorldX, int WorldY, int& SceeenX, int& SceeenY) const
{
	SceeenX = (int)((WorldX - m_shift_x) * m_zoom);
	SceeenY = (int)((WorldY - m_shift_y) * m_zoom);
}



Pos<int> Renderer::WorldToScreen(int WorldX, int WorldY) const
{
	return Pos<int>( (int)((WorldX - m_shift_x) * m_zoom), (int)((WorldY - m_shift_y) * m_zoom) );
}



void Renderer::WorldToScreen(int WorldX, int WorldY, float WorldWidth, float WorldHeight, Rect& Screen) const
{
	Screen.x = (int)((WorldX - m_shift_x) * m_zoom);
	Screen.y = (int)((WorldY - m_shift_y) * m_zoom);
	Screen.w = (int)(WorldWidth  * m_zoom);
	Screen.h = (int)(WorldHeight * m_zoom);
}



Rect Renderer::WorldToScreen(int WorldX, int WorldY, float WorldWidth, float WorldHeight) const
{
	Rect Screen;
	Screen.x = (int)((WorldX - m_shift_x) * m_zoom);
	Screen.y = (int)((WorldY - m_shift_y) * m_zoom);
	Screen.w = (int)(WorldWidth * m_zoom);
	Screen.h = (int)(WorldHeight * m_zoom);
	return Screen;
}



void Renderer::ScreenToWorld(int ScreenX, int ScreenY, int& WorldX, int& WorldY) const
{
	WorldX = (int)((float)ScreenX / m_zoom + m_shift_x);
	WorldY = (int)((float)ScreenY / m_zoom + m_shift_y);
}



Pos<float> Renderer::ScreenToWorld(int ScreenX, int ScreenY) const
{
	return Pos<float>( (float)ScreenX / m_zoom + m_shift_x, (float)ScreenY / m_zoom + m_shift_y );
}



void Renderer::ZoomAt(int ScreenX, int ScreenY, float NewZoom)
{
	int world_before_x, world_before_y;
	ScreenToWorld(ScreenX, ScreenY, world_before_x, world_before_y);

	m_zoom = NewZoom;

	int world_after_x, world_after_y;
	ScreenToWorld(ScreenX, ScreenY, world_after_x, world_after_y);

	Scroll(world_before_x - world_after_x, world_before_y - world_after_y);
}



/*Texture::Texture(const std::string& Filename, bool ConvertToRGB32)
{
#ifdef _WIN32
	strcpy_s(m_image_filename, sizeof(m_image_filename), Filename.c_str());//set this for later debugging purposes
#else
	strcpy(m_image_filename, Filename.c_str());//set this for later debugging purposes
#endif

	//SDL_Surface* surface = IMG_Load(Filename.c_str());
	SDL_Surface* surface = nullptr;
	PNG image(Filename);

	if (!surface)
		Log::Warn("Could not load: " + Filename);
	else
	{
		Log::Info("Loaded: " + Filename);

		if (ConvertToRGB32)
		{
			m_surface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_ARGB8888, NULL);
			SDL_FreeSurface(surface);
		}
		else
			m_surface = surface;
	}
}*/



unsigned int Texture::GetWidth() const
{
	if (!m_surface) return 0;
	return m_surface->w / (int)m_quality;
}



unsigned int Texture::GetHeight() const
{
	if (!m_surface) return 0;
	return m_surface->h / (int)m_quality;
}