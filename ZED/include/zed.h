#pragma once
#include <string>
#include <vector>
#include <functional>
#include <chrono>



#ifndef DLLEXPORT
#if defined(_LIB) || defined(_WINDLL)
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#ifdef _DEBUG
//#pragma comment(lib, "ZED/bin/ZEDd.lib")
#pragma comment(lib, "ZEDd.lib")
#else
#pragma comment(lib, "ZED.lib")
#endif
#endif
#endif



#ifdef _WIN32
#define NOMINMAX
//#include <windows.h>
#else
#include <cstring>
#endif


#ifdef ANDROID
#include <android/log.h>
#include <android/asset_manager.h>
#endif


#if defined(LINUX) || defined(ANDROID)

#include <stdio.h>
#include <string.h>

#define DWORD uint32_t
#define HWND  int

#define VK_TAB     0x09
#define VK_RETURN  0x0D
#define VK_SHIFT   0x10
#define VK_CONTROL 0x11
#define VK_MENU    0x12
#define VK_ESCAPE  0x1B
#define VK_SPACE   0x20
#define VK_HOME    0x24
#define VK_LEFT    0x25
#define VK_UP      0x26
#define VK_RIGHT   0x27
#define VK_DOWN    0x28
#define VK_F1      0x70
#define VK_F2      0x71
#define VK_F3      0x72
#define VK_F4      0x73
#define VK_F5      0x74
#define VK_F6      0x75
#define VK_F7      0x76
#define VK_F8      0x77
#define VK_F9      0x78
#define VK_F10     0x79
#define VK_F11     0x7A
#define VK_F12     0x7B

#define IDOK                1
#define IDCANCEL            2
#define IDABORT             3
#define IDRETRY             4
#define IDIGNORE            5
#define IDYES               6
#define IDNO                7
#define IDCLOSE             8

#define MB_OK                       0x00000000L
#define MB_OKCANCEL                 0x00000001L
#define MB_ABORTRETRYIGNORE         0x00000002L
#define MB_YESNOCANCEL              0x00000003L
#define MB_YESNO                    0x00000004L
#define MB_RETRYCANCEL              0x00000005L

#define MB_ICONERROR                0x00000010L
#define MB_ICONQUESTION             0x00000020L
#define MB_ICONWARNING              0x00000030L
#define MB_ICONINFORMATION          0x00000040L

#define fopen_s(pFile,filename,mode) ((*(pFile))=fopen((filename),(mode)))

#define swprintf_s          swprintf

#define strcpy_s(X, Y, Z) strcpy(X, Z)
#define strcat_s(X, Y, Z) strcat(X, Z)
#define sscanf_s  sscanf
#define swscanf_s swscanf

#endif