#include "../include/blob.h"
#include "../include/file.h"


using namespace ZED;



Blob::Blob(size_t Size)
{
	m_Data = (uint8_t*)malloc(Size);
	m_Size = Size;
}



Blob::Blob(File& File) : Blob(File.GetSize())
{
	File >> *this;
}



Blob::~Blob()
{
	if (!m_IsShared && m_Data)
		free(m_Data);
	m_Data = nullptr;
	m_Size = 0;
}



size_t Blob::OutputTo(void* Data, size_t BytesToRead)
{
	const size_t read = std::min(m_Size - m_ReadCursor, BytesToRead);

	memcpy(Data, &m_Data[m_ReadCursor], read);
	m_ReadCursor += read;

	return read;
}



uint8_t Blob::ReadByte()
{
	if (m_ReadCursor >= m_Size)
		return 0x00;

	uint8_t ret = m_Data[m_ReadCursor];
	m_ReadCursor++;
	return ret;
}



void Blob::Append(const void* Data, size_t BytesToWrite)
{
	if (!m_Data)
	{
		m_Data = (uint8_t*)malloc(BytesToWrite);
		m_Size = BytesToWrite;
	}

	else
	{
		const size_t bytesAvailable = m_Size - m_WriteCursor;
		if (BytesToWrite > bytesAvailable)//Need to resize buffer
		{
			uint8_t* tmp = (uint8_t*)realloc(m_Data, m_Size + (BytesToWrite - bytesAvailable));

			if (!tmp)//Out of memory?
			{
				free(m_Data);
				m_Size = 0;
				return;
			}
			else
			{
				m_Data = tmp;
				m_Size = m_Size + (BytesToWrite - bytesAvailable);
			}
		}
	}

	if (m_Data)
	{
		memcpy(&m_Data[m_WriteCursor], Data, BytesToWrite);
		m_WriteCursor += BytesToWrite;
	}
}



void Blob::Trim(size_t NewSize)
{
	m_Data = (uint8_t*)realloc(m_Data, NewSize);
	m_Size = NewSize;
}



bool Blob::InsertAt(size_t Index, const void* Data, size_t BytesToWrite)
{
	if (Index + BytesToWrite >= m_Size)
		return false;

	memcpy(&m_Data[Index], Data, BytesToWrite);
	return true;
}



void Blob::operator << (Blob& Blob)
{
	Append(Blob + Blob.m_ReadCursor, Blob.GetSize() - Blob.m_ReadCursor);
	Blob.m_ReadCursor = Blob.m_Size;
}



void Blob::operator >> (uint8_t* Data)
{
	OutputTo(Data, GetSize() - GetReadCursor());
}