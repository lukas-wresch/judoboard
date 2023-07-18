#include <random>
#include "../ZED/include/log.h"
#include "../ZED/include/sha256.h"
#include "id.h"
#include "timer.h"



using namespace Judoboard;


std::unordered_set<UUID> ID::s_UsedUUIDs;
std::mutex ID::s_Mutex;



ID::ID()
{
	m_UUID = GenerateUUID();
}



ID::~ID()
{
	s_Mutex.lock();
	s_UsedUUIDs.erase(m_UUID);
	s_Mutex.unlock();
}



void ID::Reset()
{
	s_Mutex.lock();
	s_UsedUUIDs.clear();
	s_Mutex.unlock();
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

		s_Mutex.lock();
		if (s_UsedUUIDs.find(uuid) == s_UsedUUIDs.end())//Check if there is a collision
		{
			s_UsedUUIDs.insert(uuid);
			s_Mutex.unlock();
			return uuid;//No collision, uuid can be used
		}
		s_Mutex.unlock();
	}

	return std::string();
}