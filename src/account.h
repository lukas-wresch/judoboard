#pragma once
#include <string>
#include "id.h"


namespace YAML
{
	class Emitter;
	class Node;
}



namespace Judoboard
{
	class Account : public ID
	{
	public:
		friend class Database;

		class Nonce
		{
			friend class Account;
			friend class Database;

		public:
			Nonce(uint32_t IP, uint16_t RemotePort);

			uint32_t GetIP() const { return m_IP; }
			const std::string GetChallenge() const { return m_Challenge; }
			uint32_t GetExpirationTimestamp() const { return m_TimestampExpiration; }

			const std::string GetIP2String() const;
			bool HasExpired() const;

		private:
			Nonce();

			void Refresh() const;

			std::string m_Challenge;

			uint32_t m_IP;//IP the nonce is issued to

			mutable uint32_t m_TimestampExpiration = 0;
		};

		enum class AccessLevel : int
		{
			None = 0, User, Moderator, Admin
		};

		Account(const std::string& Username, const std::string& Password, AccessLevel Permissions = AccessLevel::User) : m_Username(Username), m_Password(Password)
		{
			m_AccessLevel = Permissions;
		}
		Account(const YAML::Node& Yaml);

		const std::string GetUsername() const { return m_Username; }
		const std::string GetPassword() const { return m_Password; }
		AccessLevel GetAccessLevel() const { return m_AccessLevel; }

		void SetPassword(const std::string& NewPassword) { m_Password = NewPassword; }

		const std::string ToString(AccessLevel RequestersAccessLevel) const;

		bool Verify(const Nonce& Nonce, const std::string& Response) const;

		void operator >> (YAML::Emitter& Yaml) const;

	private:

		std::string m_Username;
		std::string m_Password;

		AccessLevel m_AccessLevel = AccessLevel::User;
	};
}