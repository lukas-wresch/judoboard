#pragma once
#include <string>
#ifdef _WIN32
#include <windows.h>
#endif
#include "core.h"
#include "renderer.h"



namespace ZED
{
	class Window
	{
	public:
		Window() = delete;
		DLLEXPORT Window(ZED::Renderer& Renderer, const std::string& Title);

		const ZED::Renderer& GetRenderer() const { return m_Renderer; }
		ZED::Renderer& GetRenderer() { return m_Renderer; }

		DLLEXPORT bool IsRunning() const;
		bool IsFullscreen() const { return m_Fullscreen; }

		DLLEXPORT void HandleEvents() const;

		DLLEXPORT void WaitTillExit() const;

		DLLEXPORT void CloseWindow();

		DLLEXPORT void ShowMouse(bool Enable = true);
		DLLEXPORT void TrapMouse(bool Enable = true);

		//Events
		std::function<void()> OnClose;

		std::function<void(int)> OnKeyDown;
		std::function<void(int)> OnKeyUp;

		std::function<void(int, int)> OnMouseMove;		
		std::function<void()> OnWheelUp;
		std::function<void()> OnWheelDown;

		std::function<void()> OnLButtonDown;
		std::function<void()> OnLButtonUp;
		std::function<void()> OnMButtonDown;
		std::function<void()> OnMButtonUp;
		std::function<void()> OnRButtonDown;
		std::function<void()> OnRButtonUp;

#ifdef _WIN32
		HWND GetWindowHandle() { return m_Hwnd; }
#endif
		
	private:
#ifdef _WIN32
		LRESULT WindowProc(UINT Message, WPARAM wParam, LPARAM lParam);
		static LRESULT CALLBACK WindowProc(HWND Hwnd, UINT Message, WPARAM wParam, LPARAM lParam);

		HWND m_Hwnd = 0;
#endif

		ZED::Renderer& m_Renderer;

		int m_Syskeys = 0;
		bool m_Fullscreen = false;
		bool m_ShouldMouseBeTrapped = false;
		bool m_IsMouseTrapped = false;
		bool m_Active = false;
		bool m_ShowCursor = true;
	};
}