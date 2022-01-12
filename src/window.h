#pragma once
#include <functional>
#include "../ZED/include/renderer_opengl.h"
//#include "../ZED/include/renderer_opengles1_1.h"
#include "../ZED/include/renderer_sdl2.h"
#include "../ZED/include/sound.h"
#include "../ZED/include/log.h"
#ifdef LINUX
#include <gtk/gtk.h>
#endif



namespace Judoboard
{
#ifdef _WIN32
	LRESULT CALLBACK WindowProc(HWND Hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
#endif


	class Window
	{
	public:
#ifdef _WIN32
		friend LRESULT CALLBACK WindowProc(HWND Hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
#else
		friend gboolean on_destroy(GtkWidget* widget, GdkEventAny* event);
		friend gboolean on_delete(GtkWidget* widget, GdkEventKey* event, gpointer data);
		friend gboolean on_configure(GtkWidget* widget, GdkEventConfigure* event, gpointer data);
#endif

		Window(const std::string& Title) : m_Renderer(IsDisplayConnected() ? new ZED::RendererOpenGL : new ZED::RendererSDL2), m_Title(Title)
		{
			if (!ZED::SoundEngine::Init())
				ZED::Log::Warn("Could not start sound engine");
		}
		//Window(const std::string& Title) : m_Renderer(new ZED::RendererOpenGLES1_1), m_Title(Title) {}
		//Window(const std::string& Title) : m_Renderer(new ZED::RendererSDL2), m_Title(Title) {}

		Window(Window&) = delete;
		Window(const Window&) = delete;
		~Window()
		{
			if (m_Renderer)
				m_Renderer->Release();
			delete m_Renderer;
			ZED::SoundEngine::Release();
		}

		static bool IsDisplayConnected();

		bool OpenWindow(std::function<bool()> MainloopCallback, std::function<void()> OnInit = nullptr);
		bool OpenWindow();
		bool HandleEvents() const;

		void CloseWindow();

		bool IsRunning() const;
		const ZED::Renderer& GetRenderer() const { return *m_Renderer; }

		void Fullscreen(bool Enabled = true) const;

		unsigned int GetDisplayWidth()  const;
		unsigned int GetDisplayHeight() const;

	private:
		std::string m_Title;

		ZED::Renderer* m_Renderer = nullptr;
		mutable bool m_IsRunning = false;

#ifdef _WIN32
		HWND m_Hwnd = 0;
#else
		GtkWindow* m_Hwnd = nullptr;
#endif
	};
}