#include "../ZED/include/core.h"
#include "club.h"



using namespace Judoboard;



Club::Club(const std::string& Name) : m_Name(Name)
{	
}



Club::Club(ZED::CSV& Stream)
{
	std::string uuid;
	Stream >> m_Name >> uuid;
	SetUUID(std::move(uuid));
}



Club::Club(const MD5::Club& Club)
{
	m_Name = Club.Name;
}



void Club::operator >> (ZED::CSV& Stream) const
{
	Stream << m_Name;
	Stream << (std::string)GetUUID();

	Stream.AddNewline();//Also needed to flush the stream
}