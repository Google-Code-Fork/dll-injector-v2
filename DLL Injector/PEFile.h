#include <Windows.h>
#ifndef PE_FILE_H
#define PE_FILE_H

class PEFile
{
public:
	PEFile(char const* filePath);
	~PEFile(void);

	void SaveFile(void) const;

	size_t GetFileSize(void) const;
	size_t GetNumberOfSections(void) const;

	void AddSection(char const* name, size_t size);
	void RemoveSection(int index);
	void RemoveSection(char const* name);
private:
	void LoadFile(void);

	char const*				m_filePath;
	char*					m_fileData;
	size_t					m_fileSize;

	IMAGE_DOS_HEADER*		m_dosHeader;
	IMAGE_NT_HEADERS*		m_ntHeaders;
	IMAGE_SECTION_HEADER*	m_firstSectionHeader;
};


#endif