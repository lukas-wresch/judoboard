#define SODIUM_STATIC
#include "../external/libsodium-stable/src/libsodium/include/sodium.h"
#include "../include/crypto.h"



#ifdef _DEBUG
#ifdef _WIN64
#pragma comment(lib, "../external/libsodium-stable/bin/x64/Debug/v142/static/libsodium.lib")
#elif _WIN32
#pragma comment(lib, "../external/libsodium-stable/bin/Win32/Debug/v142/static/libsodium.lib")
#endif

#else

#ifdef _WIN64
#pragma comment(lib, "../external/libsodium-stable/bin/x64/Release/v142/static/libsodium.lib")
#elif _WIN32
#pragma comment(lib, "../external/libsodium-stable/bin/Win32/Release/v142/static/libsodium.lib")
#endif
#endif



using namespace ZED;
using namespace Crypto;



bool g_SodiumInited = false;



bool InitLibSodium()
{
	if (!g_SodiumInited)
	{
		if (sodium_init() < 0)
			return false;//Panic! the library couldn't be initialized, it is not safe to use
		else
			g_SodiumInited = true;
	}
	return true;
}



void Crypto::GenerateRandomValue(void* pData, uint32_t Size)
{
	if (!InitLibSodium())
		return;

	randombytes_buf(pData, Size);
}



Signature SecretKey::Sign(const uint8_t* Message, uint64_t Size) const
{
	Signature sig;

	if (!InitLibSodium())
		return sig;

	uint64_t signed_message_len;
	crypto_sign_detached(sig.m_Signature, &signed_message_len, Message, Size, m_sk);
	return sig;
}



PublicKey::PublicKey(const SecretKey& SK)
{
	if (!InitLibSodium())
		return;

	crypto_scalarmult_base(this->m_pk, SK.m_sk);
}



bool PublicKey::Verify(const uint8_t* Message, uint64_t Size, const Signature& Signature) const
{
	if (!InitLibSodium())
		return false;

	if (crypto_sign_verify_detached(Signature.m_Signature, Message, Size, m_pk) != 0)
		return false;
	return true;
}



KeyPair::KeyPair()
{
	if (!InitLibSodium())
		return;

	crypto_sign_keypair(m_pk, m_sk);
}