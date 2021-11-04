#pragma once
#include "../include/window.h"
#include "../include/log.h"


#define HID_USAGE_PAGE_GENERIC         ((USHORT) 0x01)
#define HID_USAGE_GENERIC_MOUSE        ((USHORT) 0x02)


using namespace ZED;



LRESULT Window::WindowProc(UINT Message, WPARAM wParam, LPARAM lParam)
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
		if (OnClose)
			OnClose();
		//PostQuitMessage(0);//Close main process
		break;
	}

	//case WM_EXITSIZEMOVE:
	case WM_SIZE:
	{
		RECT rect;
		GetClientRect(m_Hwnd, &rect);

		auto& renderer = GetRenderer();
		renderer.Lock();
		renderer.ResetViewPort(rect.right, rect.bottom);
		renderer.Unlock();
		return 0;
	}


	case WM_INPUT:
	{
		if (m_ShouldMouseBeTrapped && !m_IsMouseTrapped)
			break;

		if (GetForegroundWindow() != m_Hwnd)
			break;

		UINT dwSize = sizeof(RAWINPUT);

		RAWINPUT lpb;
		GetRawInputData((HRAWINPUT)lParam, RID_INPUT, (void*)&lpb, &dwSize, sizeof(RAWINPUTHEADER));

		const RAWINPUT* raw = (RAWINPUT*)&lpb;

		if (raw->header.dwType == RIM_TYPEMOUSE)
		{
			if (raw->data.mouse.usFlags == MOUSE_MOVE_RELATIVE)
			{
				if (OnMouseMove)
					OnMouseMove(raw->data.mouse.lLastX, raw->data.mouse.lLastY);

			}//end of Mouse move input


			if (raw->data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN && OnLButtonDown)
				OnLButtonDown();
			if (raw->data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP && OnLButtonUp)
				OnLButtonUp();
			if (raw->data.mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN && OnRButtonDown)
				OnRButtonDown();
			if (raw->data.mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP && OnRButtonUp)
				OnRButtonUp();
			if (raw->data.mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN && OnMButtonDown)
				OnMButtonDown();
			if (raw->data.mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_UP && OnMButtonUp)
				OnMButtonUp();
			/*if (raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_4_DOWN)
				;//TODO
			if (raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_4_UP)
				;//TODO
			if (raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_5_DOWN)
				;//TODO
			if (raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_5_UP)
				;//TODO*/
		}//End of mouse input

		else if (raw->header.dwType == RIM_TYPEKEYBOARD)//Keyboard input
		{
			m_Syskeys = 0;

			/*if ((GetKeyState(VK_CONTROL) & 0x80) || (wParam == VK_CONTROL))
				m_Syskeys |= OPTIONS_MOD_CTRL;
			if ((GetKeyState(VK_MENU) & 0x80) || (wParam == VK_MENU))
				m_Syskeys |= OPTIONS_MOD_ALT;
			if ((GetKeyState(VK_SHIFT) & 0x80) || (wParam == VK_SHIFT))
				m_Syskeys |= OPTIONS_MOD_SHIFT;*/

			if (raw->data.keyboard.Flags & RI_KEY_BREAK && OnKeyUp)
				OnKeyUp(raw->data.keyboard.VKey);
			else if (OnKeyDown)
				OnKeyDown(raw->data.keyboard.VKey);
		}
		break;
	}


	case WM_MOUSEWHEEL:
	{
		auto zDelta = GET_WHEEL_DELTA_WPARAM(wParam);

		if (zDelta > 0 && OnWheelUp)
			OnWheelUp();
		else if (zDelta < 0 && OnWheelDown)
			OnWheelDown();

		break;
	}


	case WM_TIMER:
	{
		break;
	}
	}

	return DefWindowProc(m_Hwnd, Message, wParam, lParam);
}



LRESULT CALLBACK Window::WindowProc(HWND Hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	Window* pWindow = (Window*)GetWindowLongPtr(Hwnd, GWLP_USERDATA);

	if (pWindow)
		return pWindow->WindowProc(Message, wParam, lParam);
	return DefWindowProc(Hwnd, Message, wParam, lParam);
}



Window::Window(ZED::Renderer& Renderer, const std::string& Title) : m_Renderer(Renderer)
{
	SetProcessDPIAware();

	WNDCLASSEX winclass;

	wchar_t title[256] = {};
	mbstowcs_s(nullptr, title, Title.c_str(), 256);

	if (!GetClassInfoEx(GetModuleHandle(NULL), title, &winclass))
	{
		winclass.cbSize = sizeof(WNDCLASSEX);
		winclass.style = NULL;
		winclass.lpfnWndProc = WindowProc;
		winclass.cbClsExtra = 0;
		winclass.cbWndExtra = 0;
		winclass.hInstance = GetModuleHandle(NULL);
		winclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		winclass.hCursor = LoadCursor(NULL, IDC_ARROW);
		winclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
		winclass.lpszMenuName = NULL;
		winclass.lpszClassName = title;
		winclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

		if (!RegisterClassEx(&winclass))
			return;
	}

	//DWORD exStyle = WS_EX_TOPMOST;
	DWORD exStyle = NULL;

	DWORD style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;

	if (!(m_Hwnd = CreateWindowEx(exStyle,
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


	if (!m_Renderer.Init(m_Hwnd))
		return;

	SetWindowLongPtr(m_Hwnd, GWLP_USERDATA, (LONG_PTR)this);

	//Configure mouse & keyboard in raw mode
	RAWINPUTDEVICE Rid[2];
	Rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
	Rid[0].usUsage = HID_USAGE_GENERIC_MOUSE;
	Rid[0].dwFlags = RIDEV_INPUTSINK;
	Rid[0].hwndTarget = m_Hwnd;

	Rid[1].usUsagePage = 0x01;
	Rid[1].usUsage = 0x06;
	Rid[1].dwFlags = RIDEV_INPUTSINK;//Adds HID keyboard and also ignores legacy keyboard messages
	Rid[1].hwndTarget = m_Hwnd;

	Log::Info("Registering input device");

	if (!RegisterRawInputDevices(Rid, 2, sizeof(Rid[0])))
	{
		Log::Error("Error can not configure mouse & keyboard!");
		//return false;
	}


	RECT rect;
	GetClientRect(m_Hwnd, &rect);
	m_Renderer.Lock();
	m_Renderer.ResetViewPort(rect.right, rect.bottom);
	m_Renderer.Unlock();

	HandleEvents();
}



bool Window::IsRunning() const
{
	return IsWindow(m_Hwnd);
}



void Window::HandleEvents() const
{
	MSG message;
	while (PeekMessage(&message, m_Hwnd, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&message);
		DispatchMessage(&message);
	}
}



void Window::WaitTillExit() const
{
	MSG message;
	while (GetMessage(&message, m_Hwnd, 0, 0) > 0)
	{
		TranslateMessage(&message);
		DispatchMessage(&message);
	}
}



void Window::ShowMouse(bool Enable)
{
	if (m_ShowCursor == Enable)
		return;

	ShowCursor(Enable);
	m_ShowCursor = Enable;
}



void Window::TrapMouse(bool Enable)
{
	RECT client_rect;
	GetClientRect(m_Hwnd, &client_rect);

	//Convert client to screen coordinates
	POINT pos;
	RECT rect;
	pos.x = client_rect.left;
	pos.y = client_rect.top;
	ClientToScreen(m_Hwnd, &pos);
	rect.left = pos.x + 20;
	rect.top  = pos.y + 20;

	pos.x = client_rect.right;
	pos.y = client_rect.bottom;
	ClientToScreen(m_Hwnd, &pos);
	rect.right  = pos.x - 20;
	rect.bottom = pos.y - 20;

	if (Enable)
		ClipCursor(&rect);//Trap mouse
	else
		ClipCursor(NULL);//Release mouse

	m_ShouldMouseBeTrapped = Enable;
	m_IsMouseTrapped = Enable;
}



void Window::CloseWindow()
{
	m_Renderer.Lock();
	SendMessage(m_Hwnd, WM_CLOSE, 0, 0);
	m_Renderer.Unlock();
}