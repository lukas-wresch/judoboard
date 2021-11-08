#pragma once
#include <string>
#include <unordered_set>



namespace Judoboard
{
	class UUID
	{
	public:
		friend class ID;

		UUID(std::string&& UUID_) : m_UUID(std::move(UUID_)) {}
		UUID(UUID& UUID_) : m_UUID(UUID_) {}
		UUID(const UUID& UUID_) : m_UUID(UUID_) {}
		UUID(UUID&& org) noexcept : m_UUID(std::move(org.m_UUID)) {}

		explicit operator const std::string& () const { return m_UUID; }

		bool operator == (const UUID& rhs) const noexcept { return m_UUID == rhs.m_UUID; }
		bool operator != (const UUID& rhs) const noexcept { return m_UUID != rhs.m_UUID; }

		void operator = (UUID&& NewUUID) noexcept { m_UUID = std::move(NewUUID.m_UUID); }

	protected:
		void operator = (const UUID& NewUUID) noexcept { m_UUID = NewUUID.m_UUID; }

	private:
		UUID() = default;

		void operator = (std::string&& NewUUID) { m_UUID = std::move(NewUUID); }

		std::string m_UUID;
	};



	class ID
	{
	public:
		ID();
		~ID();

		static const UUID GenerateUUID();
		static void Reset();

		uint32_t GetID() const { return m_ID; }
		const UUID& GetUUID() const { return m_UUID; }

		operator const UUID& () const { return m_UUID; }

	protected:
		void SetUUID(std::string&& UUID) { m_UUID = std::move(UUID); }
		//void SetUUID(const std::string& UUID) { m_UUID = UUID; }

	private:
		void SetID(uint32_t ID) { m_ID = ID; }

		uint32_t m_ID = 0;
		UUID m_UUID;

		static uint32_t s_NextID;
		static std::unordered_set<UUID> s_UsedUUIDs;
	};
}



template<>
struct std::hash<Judoboard::UUID>
{
	std::size_t operator()(const Judoboard::UUID& uuid) const {
		return std::hash<std::string>()((std::string)uuid);
	}
};