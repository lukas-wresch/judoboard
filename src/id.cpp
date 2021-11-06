#include <random>
#include <ZED/include/log.h>
#include <ZED/include/sha512.h>
#include "id.h"
#include "timer.h"



using namespace Judoboard;


uint32_t ID::s_NextID = 1;
std::unordered_set<UUID> ID::s_UsedUUIDs;



ID::ID()
{
	if (s_NextID == 0)
	{
		ZED::Log::Error("Out of IDs! Restart the application!");
		s_NextID++;
	}

	m_ID   = s_NextID++;
	m_UUID = GenerateUUID();
}



ID::~ID()
{
	s_UsedUUIDs.erase(m_UUID);
}



void ID::Reset()
{
	s_NextID = 1;
	s_UsedUUIDs.clear();
}



const UUID ID::GenerateUUID()
{
	std::random_device rd;

	if (rd.entropy() < 1.0)
	{
		ZED::Log::Error("No entropy source found!");
		return std::string();
	}

	for (int i = 0; true; i++)//Generate ID until we don't have a collision
	{
		std::string entropyInput = std::to_string(Timer::GetTimestamp() + i);

		if (rd.entropy() < 1.0)//No entropy source. For example if we are running in the cloud
		{
			srand(Timer::GetTimestamp());
			for (int bits = 0; bits < 512; bits += 15)
				entropyInput += std::to_string(rand());
		}
		else
		{
			for (int bits = 0; bits < 512; bits += (int)rd.entropy())
				entropyInput += std::to_string(rd());
		}

		ZED::SHA512 hash(entropyInput);
		const UUID uuid(hash);

		if (s_UsedUUIDs.find(uuid) == s_UsedUUIDs.end())//Check if there is a collision
			return uuid;//No collision, uuid can be used
	}

	return std::string();
}