#ifndef PE_FILE_H
#define PE_FILE_H

#include <Windows.h>
#include <string>
#include <vector>
class PEFile
{
public:
	PEFile(char const* filePath);
	PEFile(PEFile const& rhs);
	~PEFile(void);

	PEFile& operator=(PEFile const& rhs);
	
	void Save(void) const;
	void SaveAs(char const* filePath) const;


	/* 
	Injects code into the PE File and redirects the entry point, so the code flow starts there and eventually redirects it back to the original entry point

	code = pointer to code of the infection
	size = size of the infection's code
	originalEntryPointOffset = offset, where the addr to the original entry point will be copied, so code can jump / ret back to the original code flow
	newEntryPointOffset = offset to the entry point in the code (  code + offset = new entry point), incase we inject more functions and entry point is not on the top 
	*/
	void InfectLastSection(char const* code, size_t size, DWORD originalEntryPointOffset, DWORD newEntryPointOffset = 0);
	void InfectNewSection(char const* sectionName, char const* code, size_t size, DWORD originalEntryPointOffset, DWORD newEntryPointOffset = 0);
	void AddSection(char const* name, size_t size);
	void WriteSection(char const* code, size_t size, int sectionIndex, DWORD writeOffset = 0);

	size_t GetFileSize(void) const;
	size_t GetNumberOfSections(void) const;

	class Tools
	{
	public:
		/*
		This will only work for simple-enough functions, that have one basic ret instruction.
		*/
		static size_t GetFunctionSize(void const* function);
		static size_t AlignSize(size_t size, size_t alingment);
		template <typename T> static bool Contains(T const& value, T const* array, size_t size)
		{
			for (int i = 0; i < size; i++)
			{
				if (array[i] == value)
					return true;
			}
			return false;
		}
	private:
		Tools(void);
		~Tools(void);

		static const char OPCODE_RET = 0xC3;

	};
private:
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