#include <vector>
#include "eula.h"



#define IDC_EULA_TEXT 1001
#define IDOK     1
#define IDCANCEL 2



LRESULT CALLBACK EulaWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HWND hEulaTextBox, hAcceptButton, hDeclineButton;
    bool* eula_accepted = (bool*)GetWindowLongPtr(hWnd, GWLP_USERDATA); // Retrieve EULA state

    switch (message)
    {
    case WM_CREATE:
    {
        //Create the EULA text box (scrollable)
        hEulaTextBox = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | WS_BORDER | ES_READONLY,
            10, 10, 600, 360, hWnd, (HMENU)IDC_EULA_TEXT, GetModuleHandle(NULL), NULL);

        //Load the EULA from file and set the text into the control
        HANDLE hFile = CreateFile(L"EULA.txt", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE)
        {
            DWORD fileSize = GetFileSize(hFile, NULL);
            if (fileSize != INVALID_FILE_SIZE)
            {
                std::vector<char> buffer(fileSize + 1);
                DWORD bytesRead;
                if (ReadFile(hFile, buffer.data(), fileSize, &bytesRead, NULL))
                {
                    buffer[bytesRead] = '\0';//Null-terminate the string
                    SetWindowTextA(hEulaTextBox, buffer.data());
                }
            }
            CloseHandle(hFile);
        }
        else
        {
            MessageBox(hWnd, L"EULA.txt not found!", L"Error", MB_OK | MB_ICONERROR);
            PostQuitMessage(0);//Quit if EULA not found
            return -1;
        }

        //Create the "Accept" button
        hAcceptButton = CreateWindow(L"BUTTON", L"Accept", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
            80, 390, 100, 35, hWnd, (HMENU)IDOK, GetModuleHandle(NULL), NULL);

        //Create the "Decline" button
        hDeclineButton = CreateWindow(L"BUTTON", L"Decline", WS_CHILD | WS_VISIBLE,
            640-80-100-10, 390, 100, 35, hWnd, (HMENU)IDCANCEL, GetModuleHandle(NULL), NULL);
        return 0;
    }

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK)  // Accept button clicked
        {
            if (eula_accepted)
                *eula_accepted = true;
            PostQuitMessage(0);
        }
        else if (LOWORD(wParam) == IDCANCEL)  // Decline button clicked
        {
            if (eula_accepted)
                *eula_accepted = false;
            PostQuitMessage(0);
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}



bool EULA::Display(HINSTANCE hInstance)
{
    //Register a window class for the EULA window
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = EulaWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"EULAWindowClass";
    RegisterClass(&wc);

    //Create the EULA window
    HWND hWnd = CreateWindow(L"EULAWindowClass", L"End-User License Agreement (EULA)", WS_CAPTION | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, NULL, NULL, hInstance, NULL);

    if (!hWnd)
    {
        MessageBox(NULL, L"Failed to create EULA window.", L"Error", MB_OK | MB_ICONERROR);
        return false;
    }


    RECT rc;
    GetWindowRect(hWnd, &rc);
    int winWidth  = rc.right - rc.left;
    int winHeight = rc.bottom - rc.top;

    //Get the screen dimensions
    int screenWidth  = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    //Center the window
    SetWindowPos(hWnd, NULL, 
        (screenWidth - winWidth) / 2, (screenHeight - winHeight) / 2, 
        0, 0, SWP_NOZORDER | SWP_NOSIZE);


    //Set eula_accepted to window user data
    bool eula_accepted = false;
    SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)&eula_accepted);

    ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);

    //Message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    //Return whether the user accepted the EULA or not
    return eula_accepted;
}