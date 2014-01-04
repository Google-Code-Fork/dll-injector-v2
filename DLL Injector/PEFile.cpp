#include "PEFile.h"
#include <fstream>

PEFile::PEFile(char const* filePath)
	:
	m_filePath(filePath),
	m_fileData(NULL),
	m_dosHeader(NULL),
	m_ntHeaders(NULL)
{
	LoadFile();
}


PEFile::~PEFile(void)
{
	if (m_fileData)
		delete[] m_fileData;

}
void PEFile::SaveFile(void) const
{
	std::ofstream file(m_filePath, std::ofstream::binary);
	if (!file.good())
		throw std::runtime_error("PEFile::SaveFile - Error opening a file for writing.");
	file.write(m_fileData, m_fileSize);
	if (!file.good())
		throw std::runtime_error("PEFile::SaveFile - Error writing to a file.");
}
void PEFile::LoadFile(void)
{
	std::ifstream file(m_filePath, std::ifstream::binary);
	if (!file.good())
		throw std::runtime_error("PEFile::LoadFile - File not found.");

	file.seekg(0, file.end);
	m_fileSize = file.tellg();
	file.seekg(0);

	if (m_fileSize < 1)
		throw std::runtime_error("PEFile::LoadFile - Incorrect File size.");

	m_fileData = new char[m_fileSize];
	file.read(m_fileData, m_fileSize);

	if (!file.good() || file.gcount() != m_fileSize)
		throw std::runtime_error("PEFile::LoadFile - Error reading the file.");


	m_dosHeader = reinterpret_cast<IMAGE_DOS_HEADER*>(m_fileData);
	if (m_dosHeader->e_magic != 0x5A4D)
		throw std::runtime_error("PEFile::LoadFile - DOS Header not found (Probably not a PE file).");
	m_ntHeaders = reinterpret_cast<IMAGE_NT_HEADERS*>(m_fileData + m_dosHeader->e_lfanew);
	if (m_ntHeaders->OptionalHeader.Magic != 0x010B)
		throw std::runtime_error("PEFile::LoadFile - NT Header not found (Probably a x64 file).");
	m_firstSectionHeader =  IMAGE_FIRST_SECTION(m_ntHeaders);

}
size_t PEFile::GetFileSize(void) const
{
	return m_fileSize;
}