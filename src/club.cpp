#include <ZED/include/core.h>
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



void Club::operator >> (ZED::CSV& Stream) const
{
	Stream << m_Name;	
	Stream << GetUUID();

	Stream.AddNewline();//Also needed to flush the stream
}