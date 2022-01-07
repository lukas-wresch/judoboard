#include "../include/csv.h"
#include "../include/core.h"


using namespace ZED;



void CSV::AddNewline()
{
	m_line += ZED::Core::Newline;
	Flush();
}



void CSV::Ignore()
{
	while (m_stream.peek() == ',' || m_stream.peek() == '\r' || m_stream.peek() == '\n' || m_stream.peek() == ' ')
		m_stream.ignore();
}



void CSV::Flush() const
{
	m_stream << m_line;//Flush to stream
	m_line.clear();
}



void CSV::AppendImpl(const char* Value)
{
	if (!m_stream.str().empty() || m_line.length() > 0)
		m_line += ",";
	m_line += Value;
}