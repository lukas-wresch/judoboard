#pragma once
#ifdef _WIN32
#include <windows.h>
#endif



namespace Judoboard
{
	class EULA
	{
	public:
#ifdef _WIN32
		static bool Display(HINSTANCE hInstance);
#endif
	};
}