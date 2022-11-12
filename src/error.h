#pragma once
#include <string>



namespace Judoboard
{
	class Error
	{
	public:
		enum class Type
		{
			NoError,
			NotLoggedIn, NotEnoughPermissions, InvalidID, MatNotFound,
			ApplicationShuttingDown,
			InternalError,
			ItemNotFound,
			TournamentNotOpen,
			OperationFailed,
			InvalidFormat,
			InvalidInput
		};
		
		Error() = default;
		Error(Type ErrorType) : m_Type(ErrorType) {}

		bool operator == (const Error& rhs) const {
			return m_Type == rhs.m_Type;
		}
		operator bool () const { return m_Type == Type::NoError; }

		operator std::string ();

	private:
		Type m_Type = Type::NoError;
	};
}