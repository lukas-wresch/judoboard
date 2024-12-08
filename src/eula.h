#pragma once
#ifdef _WIN32
#include <windows.h>
#endif



namespace Judoboard
{
	class EULA
	{
	public:
		static bool Display(HINSTANCE hInstance);
	};
}