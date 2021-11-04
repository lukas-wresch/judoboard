#pragma once
#include <string>
#include "core.h"
#include "blob.h"


#ifndef crypto_sign_BYTES
#define crypto_sign_BYTES 64
#endif

#ifndef crypto_sign_SECRETKEYBYTES
#define crypto_sign_SECRETKEYBYTES 64
#endif

#ifndef crypto_sign_PUBLICKEYBYTES
#define crypto_sign_PUBLICKEYBYTES 32
#endif


namespace ZED
{
	namespace Crypto
	{
		DLLEXPORT void GenerateRandomValue(void* pData, uint32_t Size);

		template<typename T>
		void GenerateRandomValue(T& Data) { GenerateRandomValue(Data, sizeof(T)); }


		struct Signature
		{
			uint8_t m_Signature[crypto_sign_BYTES] = {};
		};


		class SecretKey
		{
			friend class PublicKey;

		public:
			SecretKey(const SecretKey& SK) { memcpy(m_sk, SK.m_sk, sizeof(m_sk)); }

			DLLEXPORT Signature Sign(const uint8_t* Message, uint64_t Size) const;
			Signature Sign(const std::string& Message) const { return Sign((uint8_t*)Message.c_str(), Message.length()); }
			Signature Sign(const Blob& Message) const { return Sign(Message, Message.GetSize()); }

			void operator =(const SecretKey& SK)
			{
				memcpy(m_sk, SK.m_sk, sizeof(m_sk));
			}

		protected:
			SecretKey() {}

			uint8_t m_sk[crypto_sign_SECRETKEYBYTES] = {};
		};


		class PublicKey
		{
		public:
			PublicKey(const SecretKey& SK);

			DLLEXPORT bool Verify(const uint8_t* Message, uint64_t Size, const Signature& Signature) const;
			bool Verify(const std::string& Message, const Signature& Signature) const { return Verify((uint8_t*)Message.c_str(), Message.length(), Signature); }
			bool Verify(const Blob& Message, const Signature& Signature) const { return Verify(Message, Message.GetSize(), Signature); }

		protected:
			PublicKey() {};

			uint8_t m_pk[crypto_sign_PUBLICKEYBYTES] = {};
		};


		class KeyPair final : public PublicKey, public SecretKey
		{
		public:
			DLLEXPORT KeyPair();
			KeyPair(const SecretKey& SK) : PublicKey(SK), SecretKey(SK) {}
			KeyPair(const KeyPair& Pair) : PublicKey(Pair), SecretKey(Pair) {}

		private:
		};
	}
}