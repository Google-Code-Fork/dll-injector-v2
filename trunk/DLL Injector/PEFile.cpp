#include "PEFile.h"
#include <fstream>

PEFile::PEFile(char const* filePath)
	:
	m_fileData(NULL),
	m_dosHeader(NULL),
	m_ntHeaders(NULL)
{
	std::ifstream file(filePath, std::ifstream::binary);
	if (!file.good())
		throw std::runtime_error("PEFile::PEFile - File not found.");

	file.seekg(0, file.end);
	size_t fileSize = file.tellg();
	file.seekg(0, file.beg);

	if (fileSize < 1)
		throw std::runtime_error("PEFile::PEFile - Incorrect File size.");

	m_fileData = new char[fileSize];
	file.read(m_fileData, fileSize);

	m_dosHeader = reinterpret_cast<IMAGE_DOS_HEADER*>(m_fileData);
	m_ntHeaders = reinterpret_cast<IMAGE_NT_HEADERS*>(m_fileData + m_dosHeader->e_lfanew);
}


PEFile::~PEFile(void)
{
	if (m_fileData)
		delete[] m_fileData;

}

