#include <windows.h>
#include <thread>
#include "window.h"
#include "app.h"
#include "../ZED/include/log.h"



using namespace Judoboard;



LRESULT CALLBACK Judoboard::WindowProc(HWND Hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
		case WM_DESTROY:
		{
			//PostQuitMessage(0);//Close main process
			break;
		}

		case WM_CLOSE:
		{
			//PostQuitMessage(0);//Close main process
			break;
		}

		//case WM_EXITSIZEMOVE:
		case WM_SIZE:
		{
			if (wParam == SIZE_MINIMIZED)//Window has been minimized
				return 0;//Don't do anything, otherwise the viewport will be set to 0x0 which would make screeenshots impossible

			RECT rect;
			GetClientRect(Hwnd, &rect);

			Window* pWindow = (Window*)GetWindowLongPtr(Hwnd, GWLP_USERDATA);
			if (pWindow && pWindow->IsRunning())
			{
				auto renderer = pWindow->m_Renderer;
				renderer->Lock();
				renderer->ResetViewPort(rect.right, rect.bottom);
				renderer->Unlock();
			}
			return 0;
		}


		case WM_TIMER:
		{
			break;
		}
	}

	return DefWindowProc(Hwnd, Message, wParam, lParam);
}



bool Window::OpenWindow(std::function<bool()> MainloopCallback, std::function<void()> OnInit)
{
	if (m_IsRunning)
		return false;

	SetProcessDPIAware();

	std::thread([this, MainloopCallback, OnInit]()
	{
		WNDCLASSEX winclass;

		wchar_t title[256] = {};
		mbstowcs_s(nullptr, title, m_Title.c_str(), 256);

		if (!GetClassInfoEx(GetModuleHandle(NULL), title, &winclass))
		{
			winclass.cbSize = sizeof(WNDCLASSEX);
			winclass.style = NULL;
			winclass.lpfnWndProc = WindowProc;
			winclass.cbClsExtra = 0;
			winclass.cbWndExtra = 0;
			winclass.hInstance = GetModuleHandle(NULL);
			winclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
			//winclass.hIcon = (HICON)LoadImage(hInstance, L"AAA", IMAGE_ICON, 48, 48, 0);
			winclass.hCursor = LoadCursor(NULL, IDC_ARROW);
			//winclass.hCursor = NULL;
			winclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
			winclass.lpszMenuName = NULL;
			winclass.lpszClassName = title;
			winclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
			//winclass.hIconSm = (HICON)LoadImage(hInstance, L"AAA", IMAGE_ICON, 32, 32, 0);

			if (!RegisterClassEx(&winclass))
				return;
		}

		DWORD exstyle = WS_EX_TOPMOST;
#ifdef _DEBUG
		exstyle = NULL;
#endif
		DWORD style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;

		if (!(m_Hwnd = CreateWindowEx(exstyle,
			title,
			title,
			style,
			50, 50,
			1024, 768,
			NULL,
			NULL,
			GetModuleHandle(NULL),
			NULL)))
			return;

		if (!m_Renderer->Init(m_Hwnd))
			return;

		SetWindowLongPtr(m_Hwnd, GWLP_USERDATA, (LONG_PTR)this);

		if (OnInit)
			OnInit();

		m_IsRunning = true;

		MSG message;

		if (!MainloopCallback)
		{
			while (GetMessage(&message, m_Hwnd, 0, 0) > 0)
			{
				TranslateMessage(&message);
				DispatchMessage(&message);
			}
		}
		else
		{
			while (IsWindow(m_Hwnd))
			{
				if (!MainloopCallback())
					break;

				while (PeekMessage(&message, m_Hwnd, 0, 0, PM_REMOVE))
				{
					TranslateMessage(&message);
					DispatchMessage(&message);
				}
			}
		}

		m_IsRunning = false;
		//delete m_Renderer;
		//m_Renderer = nullptr;
	}).detach();

	return true;
}



bool Window::OpenWindow()
{
	if (IsRunning())
		return false;

	SetProcessDPIAware();

	WNDCLASSEX winclass;

	wchar_t title[256] = {};
	mbstowcs_s(nullptr, title, m_Title.c_str(), 256);

	if (!GetClassInfoEx(GetModuleHandle(NULL), title, &winclass))
	{
		winclass.cbSize = sizeof(WNDCLASSEX);
		winclass.style = NULL;
		winclass.lpfnWndProc = WindowProc;
		winclass.cbClsExtra = 0;
		winclass.cbWndExtra = 0;
		winclass.hInstance = GetModuleHandle(NULL);
		//winclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		winclass.hIcon  = (HICON)LoadImage(NULL, L"assets\\icon.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_SHARED | LR_LOADTRANSPARENT);
		//winclass.hIcon = (HICON)LoadImage(hInstance, L"AAA", IMAGE_ICON, 48, 48, 0);
		winclass.hCursor = LoadCursor(NULL, IDC_ARROW);
		//winclass.hCursor = NULL;
		winclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
		winclass.lpszMenuName = NULL;
		winclass.lpszClassName = title;
		//winclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
		winclass.hIconSm = (HICON)LoadImage(NULL, L"assets\\icon.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_SHARED | LR_LOADTRANSPARENT);
		//winclass.hIconSm = (HICON)LoadImage(hInstance, L"AAA", IMAGE_ICON, 32, 32, 0);

		if (!RegisterClassEx(&winclass))
			return false;
	}

//#ifdef _DEBUG
	DWORD exstyle = NULL;
//#else
	//DWORD exstyle = WS_EX_TOPMOST;
//#endif

	DWORD style = WS_OVERLAPPEDWINDOW;

	if (IsDisplayConnected())
		style |= WS_VISIBLE;

	if (!(m_Hwnd = CreateWindowEx(exstyle,
				title,
				title,
				style,
				50, 50,
				1024, 768,
				NULL,
				NULL,
				GetModuleHandle(NULL),
				NULL)))
		return false;

	m_Renderer->Lock();

	if (!m_Renderer->Init(m_Hwnd))
	{
		m_Renderer->Unlock();
		ZED::Log::Error("Failed to create renderer");
		return false;
	}

	SetWindowLongPtr(m_Hwnd, GWLP_USERDATA, (LONG_PTR)this);

	RECT rect;
	GetClientRect(m_Hwnd, &rect);
	
	m_Renderer->ResetViewPort(rect.right, rect.bottom);
	m_Renderer->Unlock();

	m_IsRunning = true;

	HandleEvents();

	return true;
}



bool Window::HandleEvents() const
{
	MSG message;
	while (PeekMessage(&message, m_Hwnd, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&message);
		DispatchMessage(&message);
	}

	return true;
}



bool Window::IsRunning() const
{
	return IsWindow(m_Hwnd);
}



void Window::CloseWindow()
{
	//if (!m_Renderer)
		//return;

	//m_Renderer->Lock();
	SendMessage(m_Hwnd, WM_CLOSE, 0, 0);
	m_IsRunning = false;
	//m_Renderer->Unlock();
}



void Window::Fullscreen(bool Enabled) const
{
	DWORD dwStyle = GetWindowLong(m_Hwnd, GWL_STYLE);

	if (Enabled)
	{
		SetWindowLong(m_Hwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
		SetWindowPos(m_Hwnd, NULL, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), NULL);
	}
	else
	{
		SetWindowLong(m_Hwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPos(m_Hwnd, NULL, 50, 50, 1024, 768, NULL);
	}
}



unsigned int Window::GetDisplayWidth() const
{
	return GetSystemMetrics(SM_CXSCREEN);
}



unsigned int Window::GetDisplayHeight() const
{
	return GetSystemMetrics(SM_CYSCREEN);
}



bool Window::IsDisplayConnected()
{
	return !Application::NoWindow;
}