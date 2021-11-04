#pragma once
#include "renderer_sdl2.h"


struct SDL_Window;
typedef void* SDL_GLContext;



namespace ZED
{
	class ShaderOpenGL : public Shader
	{
	public:
		ShaderOpenGL() = default;
		ShaderOpenGL(int ProgramID) : m_ProgramID(ProgramID) {}
		ShaderOpenGL(ShaderOpenGL&& rhs) noexcept : m_ProgramID(rhs.m_ProgramID) { rhs.m_ProgramID = 0; }
		ShaderOpenGL(Ref<Shader>&& rhs)
		{
			if (*rhs)
				*this = std::move(*(ShaderOpenGL*)*rhs);
		}

		void operator =(ShaderOpenGL&& rhs) noexcept
		{
			m_ProgramID = rhs.m_ProgramID;
			rhs.m_ProgramID = 0;
		}
		void operator =(Shader& rhs)
		{
			ShaderOpenGL* t = (ShaderOpenGL*)&rhs;
			m_ProgramID = t->m_ProgramID;
			t->m_ProgramID = 0;
		}

		DLLEXPORT unsigned int GetUniformLocation(const char* VariableName) const override;
		DLLEXPORT void SetUniform(unsigned int Location, unsigned int Value) const override;
		DLLEXPORT void SetUniform(unsigned int Location, unsigned int Value1, unsigned int Value2) const override;
		DLLEXPORT void SetUniform(unsigned int Location, float Value) const override;
		DLLEXPORT void SetUniform(unsigned int Location, float Value1, float Value2) const override;

		operator bool() const { return m_ProgramID > 0; }

	private:
		DLLEXPORT void SetShader() const override;

		uint32_t m_ProgramID = 0;
	};



	class RendererOpenGL : public RendererSDL2
	{
	public:
		RendererOpenGL() : RendererSDL2() {}
		~RendererOpenGL() { RendererOpenGL::Release(); }

		//Basics
		Type GetType() const override { return Type::OpenGL; }
#ifdef _WIN32
		DLLEXPORT bool Init(HWND Hwnd) override;
#else
		DLLEXPORT bool Init(GtkWindow* DrawingArea) override;
#endif
		DLLEXPORT void ResetViewPort(unsigned int width, unsigned int height) override;
		DLLEXPORT void ClearDisplay() const override;
		DLLEXPORT void SetClearColor(const ColorF& Color);
		DLLEXPORT void UpdateDisplay() const override;
		DLLEXPORT void Release() override;

		//Fonts
		DLLEXPORT Ref<Texture> RenderFont(FontSize Size, const std::string& Text, ZED::Color Textcolor) const override;

		DLLEXPORT Image TakeScreenshot() const override;
		DLLEXPORT void TakeScreenshotPNG(const char* Filename) const override;

		//Shader
		DLLEXPORT Ref<Shader> CompileShader(const char* VertexShaderSource, const char* PixelShaderSource) const override;
		Ref<Shader> CompileShader(const std::string& VertexShaderSource, const std::string& PixelShaderSource) const {
			return CompileShader(VertexShaderSource.c_str(), PixelShaderSource.c_str());
		}
		DLLEXPORT void SetShader(const Shader& NewShader) const override;

		//Render calls
		DLLEXPORT void FillRect(const Rect& dstrect, unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255) const override;

		DLLEXPORT void Render(const Texture& tex, int x, int y, bool render_hit = false) const override;
		DLLEXPORT void RenderPartial(const Texture& tex, int x, int y, int w, int h, bool render_hit = false) const override;
		DLLEXPORT void RenderRotated(const Texture& tex, int x, int y, bool render_hit = false) const override;

		DLLEXPORT void RenderTransformed(const Texture& tex, int x, int y) const override;
		DLLEXPORT void RenderTransformedPartial(const Texture& tex, int x, int y, int w, int h) const override;
		DLLEXPORT void RenderTransformedRotated(const Texture& tex, int x, int y) const override;

	private:
		SDL_GLContext m_glcontext = nullptr;//OpenGL context

		unsigned int m_PBO[2] = {};//OpenGL Pixel buffer objects

		mutable const Shader* m_CurrentShader = nullptr;
	};


	class TextureOpenGL : public TextureSDL2
	{
	public:
		DLLEXPORT TextureOpenGL(const std::string& Filename);
		TextureOpenGL(int w, int h) : TextureSDL2(w, h), m_Width(w), m_Height(h) {}
		DLLEXPORT TextureOpenGL(const TextureSDL2& Surface);
		TextureOpenGL() : TextureSDL2() {}
		//TextureOpenGL(TextureOpenGL& Original);
		TextureOpenGL(const TextureOpenGL& Original) = delete;
		virtual ~TextureOpenGL() { TextureOpenGL::Unload(); };

		TextureOpenGL(TextureOpenGL&& Original) noexcept
		{
			m_surface = Original.m_surface;
			m_gl_texture = Original.m_gl_texture;

			Original.m_surface = nullptr;
			Original.m_gl_texture = 0;
		}

		void operator =(TextureOpenGL&& Original) noexcept
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
		//virtual void CloneTo(Texture& Copy) const = 0;
		/*virtual void ConvertToGrayscale();
		virtual void MultiplyWith(float Value);
		virtual void Recolor(unsigned char r, unsigned char g, unsigned char b, float alpha = 0.9f);
		//virtual bool Compare(const Texture& Surface) const = 0;
		virtual void FlipSurface();
		virtual void MirrorSurface();*/

	protected:
		DLLEXPORT bool LoadTexture() const override;

	private:
		mutable unsigned int m_gl_texture = 0;

		uint32_t m_Width  = 0;
		uint32_t m_Height = 0;
	};
}