#pragma once
#include "core.h"
#include "socket_tcp.h"



typedef struct ssl_ctx_st SSL_CTX;
typedef struct ssl_st SSL;



namespace ZED
{
	class DLLEXPORT SocketSSL : public SocketTCP
	{
	public:
		SocketSSL() = default;
		SocketSSL(const SocketTCP& Original);
		SocketSSL(const SocketSSL& Original) = delete;
		SocketSSL(SocketSSL&& Original) noexcept = delete;

		~SocketSSL();

		void operator =(const SocketSSL& rhs) = delete;

		bool Connect(const char* Host, uint16_t Port) override;
		bool Connect(const std::string& Host, uint16_t Port) { return Connect(Host.c_str(), Port); }

		virtual bool IsConnected() const override {
			return SocketTCP::IsConnected() && m_SSL_Context && m_SSL;
		}

		virtual bool Listen(uint16_t Port) override;
		virtual Socket* AcceptClient() const override;

		//const char* GetBuffer() const { return m_Buffer; }
		//uint32_t GetBufferLength() const { return m_BufferPosition; }
		//void ClearBuffer() { m_BufferPosition = 0; }
		std::string Buffer2String()
		{
			std::string ret(GetBuffer(), GetBufferLength());
			ClearBuffer();
			return ret;
		}

		bool Send(const void* Data, uint32_t Size);
		template <typename T>
		bool Send(const T& Data) { return Send((const char*)&Data, sizeof(T)); }

		bool Recv();


		operator std::string ()
		{
			return Buffer2String();
		}


		static constexpr uint32_t BufferSize = 4096;

	private:
		bool CreateSSLContext();

		SSL_CTX* m_SSL_Context = nullptr;
		SSL* m_SSL = nullptr;
	};
}