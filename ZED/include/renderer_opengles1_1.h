#pragma once
#include "renderer.h"



typedef void* EGLContext;
typedef void* EGLDisplay;
typedef void* EGLSurface;



namespace ZED
{
	class RendererOpenGLES1_1 : public Renderer
	{
	public:
		RendererOpenGLES1_1() : Renderer() {}
		~RendererOpenGLES1_1() { RendererOpenGLES1_1::Release(); }

		//Basics
		Type GetType() const override { return Type::OpenGLES1_1; }
#ifdef _WIN32
		DLLEXPORT bool Init(HWND Hwnd) override;
#else
		DLLEXPORT bool Init(GtkWindow* Window) override;
#endif
		DLLEXPORT void ResetViewPort(unsigned int width, unsigned int height) override;
		DLLEXPORT void ClearDisplay() const override;
		DLLEXPORT void SetClearColor(const ColorF& Color);
		DLLEXPORT void UpdateDisplay() const override;
		DLLEXPORT void Release() override;

		//Fonts
		DLLEXPORT Ref<Texture> RenderFont(FontSize Size, const std::string& Text, ZED::Color Textcolor) const override { return nullptr; }

		DLLEXPORT void TakeScreenshotPNG(const char* Filename) const override {}

		//Shader
		DLLEXPORT Ref<Shader> CompileShader(const char* VertexShaderSource, const char* PixelShaderSource) const override { return nullptr; }
		Ref<Shader> CompileShader(const std::string& VertexShaderSource, const std::string& PixelShaderSource) const {
			return CompileShader(VertexShaderSource.c_str(), PixelShaderSource.c_str());
		}
		DLLEXPORT void SetShader(const Shader& NewShader) const override {}

		//Render calls
		DLLEXPORT void FillRect(const Rect& dstrect, unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255) const override;

		DLLEXPORT void Render(const Texture& tex, int x, int y, bool render_hit = false) const override {}
		DLLEXPORT void RenderPartial(const Texture& tex, int x, int y, int w, int h, bool render_hit = false) const override {}
		DLLEXPORT void RenderRotated(const Texture& tex, int x, int y, bool render_hit = false) const override {}

		DLLEXPORT void RenderTransformed(const Texture& tex, int x, int y) const override {}
		DLLEXPORT void RenderTransformedPartial(const Texture& tex, int x, int y, int w, int h) const override {}
		DLLEXPORT void RenderTransformedRotated(const Texture& tex, int x, int y) const override {}

	private:
		EGLContext m_glcontext = nullptr;//OpenGL ES context
		EGLDisplay m_Display = nullptr;
		EGLSurface m_Surface = nullptr;

		unsigned int m_PBO[2] = {};//OpenGL Pixel buffer objects
	};


	class TextureOpenGLES1_1 : public Texture
	{
	public:
		//DLLEXPORT TextureOpenGLES1_1(const std::string& Filename);
		TextureOpenGLES1_1(int w, int h) : m_Width(w), m_Height(h) {}
		//DLLEXPORT TextureOpenGLES1_1(const TextureSDL2& Surface);
		TextureOpenGLES1_1() = default;
		//TextureOpenGL(TextureOpenGL& Original);
		TextureOpenGLES1_1(const Image& Image);
		TextureOpenGLES1_1(const TextureOpenGLES1_1& Original) = delete;
		virtual ~TextureOpenGLES1_1() { TextureOpenGLES1_1::Unload(); };

		TextureOpenGLES1_1(TextureOpenGLES1_1&& Original) noexcept
		{
			m_surface = Original.m_surface;
			m_gl_texture = Original.m_gl_texture;

			Original.m_surface = nullptr;
			Original.m_gl_texture = 0;
		}

		void operator =(TextureOpenGLES1_1&& Original) noexcept
		{
			Unload();

			m_surface = Original.m_surface;
			m_gl_texture = Original.m_gl_texture;

			Original.m_surface = nullptr;
			Original.m_gl_texture = 0;
		}

		//Basics
		virtual Type GetType() const { return Type::OpenGL; }
		DLLEXPORT void Unload() override;
		virtual bool IsTextureLoaded() const override { return m_gl_texture; }

		uint32_t GetWidth()  const override { return m_Width; }
		uint32_t GetHeight() const override { return m_Height; }

		//Texture manipulation
		DLLEXPORT void BindTexture(unsigned int Slot = 0) const override;

	protected:
		DLLEXPORT bool LoadTexture() const override { return true; }

	private:
		mutable unsigned int m_gl_texture = 0;

		uint32_t m_Width  = 0;
		uint32_t m_Height = 0;
	};
}