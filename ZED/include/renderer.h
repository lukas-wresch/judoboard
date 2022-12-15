#pragma once
#include <string>
#include <mutex>

#include "core.h"
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif



typedef struct _GtkWidget GtkWidget;
typedef struct _GtkWindow GtkWindow;
struct SDL_Surface;



namespace ZED
{
	class Texture;
	class Image;


	inline bool isz(float num) { return (num < 0.00001f && num > -0.00001f); }
	inline bool is1(float num) { return (num < 1.00001f && num >  0.99999f); }



	enum class Type
	{
		None = 0,
		SDL2 = 1,
#ifndef NO_OPENGL
		OpenGL = 2,
		OpenGLES1_1 = 2,//OpenGL ES 1.1
#endif
#ifndef NO_DX
		DirectX = 3
#endif
	};


	enum class TextureQuality : uint32_t
	{
		SD = 1, HD = 2, UHD = 4
	};


	enum class FontSize
	{
		Small,
		Middle,
		Large,
		Huge,
		Gigantic,
		Gigantic2,
	};


	template <typename T>
	struct Pos
	{
		Pos(T x_, T y_) : x(x_), y(y_) {}

		T x, y;
	};


	struct Rect
	{
		Rect() = default;
		Rect(uint32_t x_, uint32_t y_, uint32_t w_, uint32_t h_)
		{
			x = x_;
			y = y_;
			w = w_;
			h = h_;
		}

		bool Contains(uint32_t X, uint32_t Y) const
		{
			return x <= X && X <= x + w && y <= Y && Y <= y + h;
		}

		uint32_t x, y;
		uint32_t w, h;
	};



	template <typename T>
	struct Ref
	{
		Ref() = default;
		Ref(T* data_) : data(data_) {}
		Ref(Ref&) = delete;
		Ref(const Ref&) = delete;
		Ref(Ref<T>&& New) noexcept : data(New.data)
		{
			New.data = nullptr;
		}

		~Ref()
		{
			delete data;
			data = nullptr;
		}

		/*void operator =(Ref<T>& New)
		{
			delete data;
			data = New.data;
			New.data = nullptr;
		}*/

		void Reset(T* NewReference = nullptr)\
		{
			data = NewReference;
		}

		void operator =(Ref<T>&& New) noexcept
		{
			if (this != &New)
			{
				delete data;
				data = New.data;
				New.data = nullptr;
			}
		}

		/*void operator =(T* data_)
		{
			delete data;
			data = data_;
		}*/

		T* operator *() { return data; }
		T* operator ->() { return data; }
		const T* operator ->() const { return data; }

		operator T* () { return data; }
		operator const T* () const { return data; }

		T* data = nullptr;
	};



	class DLLEXPORT Shader
	{
		friend class Renderer;
		friend class RendererOpenGL;

	public:
		virtual uint32_t GetUniformLocation(const char* VariableName) const = 0;
		uint32_t GetUniformLocation(const std::string& VariableName) const { return GetUniformLocation(VariableName.c_str()); }
		virtual void SetUniform(unsigned int Location, unsigned int Value) const = 0;
		virtual void SetUniform(unsigned int Location, unsigned int Value1, unsigned int Value2) const = 0;
		virtual void SetUniform(unsigned int Location, float Value) const = 0;
		virtual void SetUniform(unsigned int Location, float Value1, float Value2) const = 0;

	private:
		virtual void SetShader() const = 0;
	};



	class Renderer
	{
	public:
		Renderer()
		{
			if (!s_pInstance)
				s_pInstance = this;
			else
				s_pInstance = nullptr;//Set to nullpointer to make GetInstance() unavailable with multiple renderers
		}
		/*~Renderer()
		{
			if (s_pInstance)
				s_pInstance->Release();
		}*/
		virtual ~Renderer() {}

		[[nodiscard]]
		static Renderer& GetInstance() { return *s_pInstance; }
		void SetAsDefaultInstance() { s_pInstance = this; }

		DLLEXPORT Ref<Texture> CreateTexture() const;
		DLLEXPORT Ref<Texture> CreateTexture(int w, int h) const;
		DLLEXPORT Ref<Texture> CreateTexture(const char* Filename) const;
		DLLEXPORT Ref<Texture> CreateTexture(const Image& Image) const;

		void Lock()   const { m_mutex.lock(); }
		void Unlock() const { m_mutex.unlock(); }

		//Basics
		virtual Type GetType() const { return Type::None; }
#ifdef _WIN32
		virtual bool Init(HWND Hwnd) = 0;
#else
		virtual bool Init(GtkWindow* DrawingArea) = 0;
#endif
		DLLEXPORT virtual void ResetViewPort(uint32_t Width, uint32_t Height) = 0;
		DLLEXPORT virtual void ClearDisplay()  const = 0;
		DLLEXPORT virtual void UpdateDisplay() const = 0;
		DLLEXPORT virtual void Release() = 0;

		//Fonts
		virtual bool InitFonts(const char* FontFile, int Small = 7, int Middle = 10, int Large = 20, int Huge = 32, int Gigangtic = -1, int Gigantic2 = -1) const { return false; }
		virtual Ref<Texture> RenderFont(FontSize Size, const std::string& Text, ZED::Color Textcolor) const { return nullptr; }

		virtual Image TakeScreenshot() const = 0;
		virtual void TakeScreenshotPNG(const char* Filename) const {};
		void TakeScreenshotPNG(const std::string& Filename) const { return TakeScreenshotPNG(Filename.c_str()); }

		//Statistics
		[[nodiscard]]
		int  GetNumDrawCalls() const { return m_DrawCalls; }
		void ResetNumDrawCalls() const { m_DrawCalls = 0; }

		[[nodiscard]]
		unsigned int GetWidth() const { return m_screen_w; }
		[[nodiscard]]
		unsigned int GetHeight() const { return m_screen_h; }

		//Scrolling / Zoom
		DLLEXPORT void GetScroll(int& X, int& Y) const;
		DLLEXPORT void SetScroll(int X, int Y);
		DLLEXPORT void Scroll(int X, int Y);
		DLLEXPORT void WorldToScreen(int WorldX, int WorldY, int& ScreenX, int& ScreenY) const;
		DLLEXPORT Pos<int> WorldToScreen(int WorldX, int WorldY) const;
		DLLEXPORT void WorldToScreen(int WorldX, int WorldY, float WorldWidth, float WorldHeight, Rect& Screen) const;
		DLLEXPORT Rect WorldToScreen(int WorldX, int WorldY, float WorldWidth, float WorldHeight) const;
		DLLEXPORT void ScreenToWorld(int ScreenX, int ScreenY, int& WorldX, int& WorldY) const;
		DLLEXPORT Pos<float> ScreenToWorld(int ScreenX, int ScreenY) const;
		DLLEXPORT float GetZoom() const { return m_zoom; }
		DLLEXPORT void SetZoom(float Zoom) { m_zoom = Zoom; }
		DLLEXPORT void ZoomAt(int ScreenX, int ScreenY, float NewZoom);

		//Shader
		DLLEXPORT Ref<Shader> CompileShader(const std::string& Filename) const;
		virtual Ref<Shader> CompileShader(const char* VertexShaderSource, const char* PixelShaderSource) const = 0;
		Ref<Shader> CompileShader(const std::string& VertexShaderSource, const std::string& PixelShaderSource) const {
			return CompileShader(VertexShaderSource.c_str(), PixelShaderSource.c_str());
		}
		virtual void SetShader(const Shader& new_shader) const {};

		//Render calls
		virtual void FillRect(const Rect& dstrect, unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255) const = 0;

		virtual void Render(const Texture& tex, int x, int y, bool render_hit = false) const = 0;
		virtual void RenderPartial(const Texture& tex, int x, int y, int w, int h, bool render_hit = false) const = 0;
		virtual void RenderRotated(const Texture& tex, int x, int y, bool render_hit = false) const = 0;

		virtual void RenderTransformed(const Texture& tex, int x, int y) const = 0;
		virtual void RenderTransformedPartial(const Texture& tex, int x, int y, int w, int h) const = 0;
		virtual void RenderTransformedRotated(const Texture& tex, int x, int y) const = 0;

		void RenderTransformed(const Texture* tex, int x, int y) const { if (tex) RenderTransformed(*tex, x, y); }
		void RenderTransformedPartial(const Texture* tex, int x, int y, int w, int h) const { if (tex) RenderTransformedPartial(*tex, x, y, w, h); }
		void RenderTransformedRotated(const Texture* tex, int x, int y) const { if (tex) RenderTransformedRotated(*tex, x, y); }

	protected:
		unsigned int m_screen_w = 0;
		unsigned int m_screen_h = 0;

		mutable unsigned int m_DrawCalls = 0;

		//Scrolling / Zoom
		int m_shift_x = 0;
		int m_shift_y = 0;
		float m_zoom = 1.0f;

	private:
		DLLEXPORT static Renderer* s_pInstance;

		mutable std::recursive_mutex m_mutex;
	};


	class Texture
	{
		friend class Renderer;
		friend class RendererSDL2;
		friend class TextureSDL2;
		friend class RendererOpenGL;

	public:
		//Texture(const std::string& Filename, bool ConvertToRGB32);
		Texture() {}
		Texture(Texture&& Original) noexcept {}
		virtual ~Texture() {};

		virtual void operator =(Texture&& Original) noexcept {}

		//Basics
		virtual Type GetType() const { return Type::None; }
		virtual void Unload() {}
		const std::string GetFilename() const { return std::string(m_image_filename); }
		virtual uint32_t GetWidth()  const = 0;//Returns the resolution-independent width of the texture. So, the texture width in SD quality. Returns 0 if surface is not loaded
		virtual uint32_t GetHeight() const = 0;//Returns the resolution-independent height of the texture. So, the texture height in SD quality. Returns 0 if surface is not loaded

		float GetSizeX() const { return m_sizeX; }
		float GetSizeY() const { return m_sizeY; }
		unsigned char GetAlpha() { return m_alpha; }

		virtual void SetSize(float size_) { m_sizeX = m_sizeY = size_; }
		virtual void SetSizeX(float size_) { m_sizeX = size_; }
		virtual void SetSizeY(float size_) { m_sizeY = size_; }
		virtual void SetAngle(float angle_) { m_angle = angle_; }
		virtual void SetAlpha(unsigned char alpha_) { m_alpha = alpha_; }

		bool IsLoaded() const { return m_surface; }
		virtual bool IsTextureLoaded() const { return m_surface; }

		//Texture manipulation
		virtual void BindTexture(unsigned int Slot = 0) const {};
		virtual void CreateCircle() {};
		virtual void CreateGradient(int r1, int g1, int b1, int r2, int g2, int b2) {}
		virtual void ConvertToGrayscale() {}
		virtual void MultiplyWith(float Value) {}
		virtual void Recolor(unsigned char r, unsigned char g, unsigned char b, float alpha = 0.9f) {}
		virtual bool Compare(const Texture& Surface) const { return false; };
		virtual void FlipSurface() {}
		virtual void MirrorSurface() {}

		//Render calls
#ifndef ZED_MULTIPLE_RENDERER
		void Render(int x, int y, bool render_hit = false) const { Renderer::GetInstance().Render(*this, x, y, render_hit); }
		void RenderPartial(int x, int y, int w, int h, bool render_hit = false) const { Renderer::GetInstance().RenderPartial(*this, x, y, w, h, render_hit); }
		void RenderRotated(int x, int y, bool render_hit = false) const { Renderer::GetInstance().RenderRotated(*this, x, y, render_hit); }

		void RenderTransformed(int x, int y) const { Renderer::GetInstance().RenderTransformed(*this, x, y); }
		void RenderTransformedPartial(int x, int y, int w, int h) const { Renderer::GetInstance().RenderTransformedPartial(*this, x, y, w, h); }
		void RenderTransformedRotated(int x, int y) const { Renderer::GetInstance().RenderTransformedRotated(*this, x, y); }
#endif

	protected:
		virtual bool LoadTexture() const { return true; }

		const SDL_Surface* GetSurface() const { return m_surface; }
		SDL_Surface* GetSurface() { return m_surface; }

		mutable SDL_Surface* m_surface = nullptr;

		float m_sizeX = 1.0f;
		float m_sizeY = 1.0f;
		float m_angle = 0.0f;
		unsigned char m_alpha = 255;

		virtual SDL_Surface* GetRotatedSurface() const { return nullptr; }


		char m_image_filename[128] = {};
		TextureQuality m_quality = TextureQuality::SD;//Usually 1 (16x16), 2 for HD (32x32), 4 for 4K (64x64)
	};
}



inline DLLEXPORT ZED::Renderer* ZED::Renderer::s_pInstance = nullptr;