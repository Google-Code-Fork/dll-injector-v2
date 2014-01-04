#include "PEFile.h"
#include <fstream>

PEFile::PEFile(char const* filePath)
	:
	m_filePath(filePath),
	m_fileBuffer(NULL),
	m_fileSize(0),
	m_bufferSize(0),
	m_dosHeader(NULL),
	m_ntHeaders(NULL),
	m_firstSectionHeader(NULL)
{
	LoadFile();
	UpdateHeaderPointers();
}


PEFile::~PEFile(void)
{
	if (m_fileBuffer)
		delete[] m_fileBuffer;

}
void PEFile::SaveFile(void) const
{
	std::ofstream file(m_filePath, std::ofstream::binary);
	if (!file.good())
		throw std::runtime_error("PEFile::SaveFile - Error opening a file for writing.");
	file.write(m_fileBuffer, m_fileSize);
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

	m_bufferSize = m_fileSize * 2;
	m_fileBuffer = new char[m_bufferSize];
	file.read(m_fileBuffer, m_fileSize);

	if (!file.good() || file.gcount() != m_fileSize)
		throw std::runtime_error("PEFile::LoadFile - Error reading the file.");
}
void PEFile::ExpandLastSection(size_t size)
{
	size_t alignedSize = AlignSize(size, m_ntHeaders->OptionalHeader.SectionAlignment);
	if (m_fileSize + alignedSize > m_bufferSize)
		ReallocateBuffer((m_fileSize + alignedSize) * 2);


	IMAGE_SECTION_HEADER* lastSection = m_firstSectionHeader + m_ntHeaders->FileHeader.NumberOfSections - 1;

	lastSection->SizeOfRawData = alignedSize;
	lastSection->Misc.VirtualSize = alignedSize;
}
void PEFile::ReallocateBuffer(size_t size)
{
	char* newBuffer = new char[size];
	memcpy(newBuffer, m_fileBuffer, m_fileSize);
	try
	{
		UpdateHeaderPointers(); // Should never really throw here, but just incase, to avoid memory leak
	}
	catch (std::runtime_error& e)
	{
		delete[] newBuffer;
		throw std::runtime_error("PEFile::ReallocateBuffer - Could not recognize headers after reallocation.");
	}
	delete[] m_fileBuffer;
	m_fileBuffer = newBuffer;
	m_bufferSize = size;
}
void PEFile::UpdateHeaderPointers(void)
{
	m_dosHeader = reinterpret_cast<IMAGE_DOS_HEADER*>(m_fileBuffer);
	if (m_dosHeader->e_magic != 0x5A4D)
		throw std::runtime_error("PEFile::UpdateHeaderPointers - DOS Header not found (Probably not a PE file).");
	m_ntHeaders = reinterpret_cast<IMAGE_NT_HEADERS*>(m_fileBuffer + m_dosHeader->e_lfanew);
	if (m_ntHeaders->OptionalHeader.Magic != 0x010B)
		throw std::runtime_error("PEFile::UpdateHeaderPointers - NT Header not found (Probably a x64 file).");
	m_firstSectionHeader = IMAGE_FIRST_SECTION(m_ntHeaders);
}
size_t PEFile::GetFileSize(void) const
{
	return m_fileSize;
}
size_t PEFile::GetNumberOfSections(void) const
{
	return m_ntHeaders->FileHeader.NumberOfSections;
}
size_t PEFile::AlignSize(size_t size, size_t alignment)
{
	size_t alignedSize = alignment;
	while (alignedSize < size)
	{
		alignedSize += alignment;
	}
	return alignedSize;
}