#include <Windows.h>
#ifndef PE_FILE_H
#define PE_FILE_H

class PEFile
{
public:
	PEFile(char const* fileName);
	~PEFile(void);

private:
	char*				m_fileData;
	size_t				m_fileSize;

	IMAGE_DOS_HEADER*	m_dosHeader;
	IMAGE_NT_HEADERS*	m_ntHeaders;
};


#endif