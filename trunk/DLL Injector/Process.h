#ifndef PROCESS_H
#define PROCESS_H
#include <Windows.h>
#include <vector>

class Process
{
public:
	Process(char const* processName);
	~Process(void);

	operator HANDLE(void) const;

	LPVOID AllocMemory(size_t size);
	void FreeMemory(LPVOID address);

	void WriteMemory(LPVOID address, LPCVOID buffer, size_t size) const;
	void ReadMemory(LPCVOID address, LPVOID buffer, size_t size) const;
	
	DWORD CallFunction(LPCVOID address, LPVOID arg);

	HANDLE GetHandle(void) const;
	static DWORD	GetProcessIDByName(char const* processName);
private:
	HANDLE				m_handle;
	std::vector<LPVOID>	m_allocs;
};
#endif