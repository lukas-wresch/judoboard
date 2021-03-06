#include <random>
#include "../ZED/include/log.h"
#include "../ZED/include/sha512.h"
#include "account.h"
#include "timer.h"
#define YAML_CPP_STATIC_DEFINE
#include "yaml-cpp/yaml.h"



using namespace Judoboard;



Account::Nonce::Nonce(uint32_t IP, uint16_t RemotePort) : m_IP(IP)
{
	std::random_device rd;

	std::string entropyInput = std::to_string(m_IP) + std::to_string(RemotePort) + std::to_string(Timer::GetTimestamp());

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

	ZED::SHA512 hash(entropyInput);
	m_Challenge = hash;

	Refresh();
}



const std::string Account::Nonce::GetIP2String() const
{
	int a = (m_IP & 0xFF000000) >> 24;
	int b = (m_IP & 0x00FF0000) >> 16;
	int c = (m_IP & 0x0000FF00) >>  8;
	int d = (m_IP & 0x000000FF) >>  0;

	return std::to_string(a) + "." + std::to_string(b) + "." + std::to_string(c) + "." + std::to_string(d);
}



void Account::Nonce::Refresh() const
{
	m_TimestampExpiration = Timer::GetTimestamp() + 5 * 60 * 1000;//5 minutes
}



bool Account::Nonce::HasExpired() const
{
	return Timer::GetTimestamp() > m_TimestampExpiration;
}



Account::Account(const YAML::Node& Yaml)
{
	if (Yaml["uuid"])
		SetUUID(Yaml["uuid"].as<std::string>());
	if (Yaml["username"])
		m_Username = Yaml["username"].as<std::string>();
	if (Yaml["password"])
		m_Password = Yaml["password"].as<std::string>();
	if (Yaml["access_level"])
		m_AccessLevel = (AccessLevel)Yaml["access_level"].as<int>();
}



bool Account::Verify(const Nonce& Nonce, const std::string& Response) const
{
	return ZED::SHA512(m_Username + Nonce.GetChallenge() + m_Password) == Response;
}



void Account::operator >> (YAML::Emitter& Yaml) const
{
	Yaml << YAML::BeginMap;
	Yaml << YAML::Key << "uuid"      << YAML::Value << (std::string)GetUUID();
	Yaml << YAML::Key << "username" << YAML::Value << m_Username;
	Yaml << YAML::Key << "password"  << YAML::Value << m_Password;
	Yaml << YAML::Key << "access_level"    << YAML::Value << (int)m_AccessLevel;
	Yaml << YAML::EndMap;
}



const std::string Account::ToString(AccessLevel RequestersAccessLevel) const
{
	if (RequestersAccessLevel == AccessLevel::Admin)
		return m_Username + "," + m_Password + "," + std::to_string((int)m_AccessLevel);
	else if (RequestersAccessLevel == AccessLevel::Moderator)
		return m_Username + ",********," + std::to_string((int)m_AccessLevel);
	return "";
}