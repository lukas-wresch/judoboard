#pragma once
#ifndef IMPORT
#if defined(_LIB) || defined(_WINDLL)
#define IMPORT __declspec(dllexport)
#else
#define IMPORT
#endif
#endif



class License
{
public:
	enum class State
	{
		Unknown        = 0,
		NotInitialized = 1,
		FileNotExist   = 2,
		WrongSignature = 3,
		WrongHash      = 4,
		Expired        = 5,
		Valid          = 6
	};

	enum class Type
	{
		Unknown      = 0,
		Basic        = 1,
		Standard     = 2,
		Professionel = 3
	};


	IMPORT static bool Check(void* app);

	IMPORT static char* GetUserID();
	IMPORT static State GetLicenseState();
	IMPORT static Type  GetLicenseType();
	IMPORT static char* GetLicenseExpiration();

	IMPORT static void Sign(const char* UserID, Type Type, const char* Expiration, const char* Name);
};