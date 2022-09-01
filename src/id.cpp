#include <random>
#include "../ZED/include/log.h"
#include "../ZED/include/sha256.h"
#include "id.h"
#include "timer.h"



using namespace Judoboard;


std::unordered_set<UUID> ID::s_UsedUUIDs;



ID::ID()
{
	m_UUID = GenerateUUID();
}



ID::ID(ZED::Blob& Stream)
{
	ZED::SHA256 hash;
	Stream >> hash;
	m_UUID = hash;
}



ID::~ID()
{
	s_UsedUUIDs.erase(m_UUID);
}



void ID::Reset()
{
	s_UsedUUIDs.clear();
}



const UUID ID::GenerateUUID()
{
	std::random_device rd;

	for (int i = 0; true; i++)//Generate ID until we don't have a collision
	{
		std::string entropyInput = std::to_string(Timer::GetTimestamp() + i);

		if (rd.entropy() < 1.0)//No entropy source. For example if we are running in the cloud
		{
			//ZED::Log::Debug("No entropy source found!");

			//srand(Timer::GetTimestamp());
			for (int bits = 0; bits < 512; bits += 15)
				entropyInput += std::to_string(rand());
		}
		else
		{
			for (int bits = 0; bits < 512; bits += (int)rd.entropy())
				entropyInput += std::to_string(rd());
		}

		ZED::SHA256 hash(entropyInput);
		const UUID uuid(hash);

		if (s_UsedUUIDs.find(uuid) == s_UsedUUIDs.end())//Check if there is a collision
		{
			s_UsedUUIDs.insert(uuid);
			return uuid;//No collision, uuid can be used
		}
	}

	return std::string();
}