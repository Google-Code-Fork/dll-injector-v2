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

	HANDLE GetHandle(void) const;
	static DWORD	GetProcessIDByName(char const* processName);
private:
	HANDLE				m_handle;
	std::vector<LPVOID>	m_allocs;
};
#endif