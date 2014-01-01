#include "Process.h"
#include <TlHelp32.h>
Process::Process(char const* processName)
	:
	m_handle(INVALID_HANDLE_VALUE)
{
	DWORD procID = Process::GetProcessIDByName(processName);
	m_handle = OpenProcess(PROCESS_ALL_ACCESS, false, procID);

	if (m_handle == NULL)
		throw std::runtime_error("Process::Process - Could not open the process");
}


Process::~Process(void)
{
	for (LPVOID& address : m_allocs)
		FreeMemory(address);

	if (m_handle != INVALID_HANDLE_VALUE && m_handle != NULL)
		CloseHandle(m_handle);
}

LPVOID Process::AllocMemory(size_t size)
{
	LPVOID address = VirtualAllocEx(m_handle, NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

	if (address == NULL)
		throw std::runtime_error("Process::AllocMemory - Memory allocation failed");

	m_allocs.push_back(address); // store memory addr and keep track of alloc, incase caller forgets to cleanup (we do it in dtor)
	return address;
}
void Process::FreeMemory(LPVOID address)
{
	std::vector<LPVOID>::iterator position = std::find(m_allocs.begin(), m_allocs.end(), address);
	if (position == m_allocs.end())
		throw std::runtime_error("Process::FreeMemory - Allocation on this address not found.");

	bool success = VirtualFreeEx(m_handle, address, 0, MEM_RELEASE);

	if (!success)
		throw std::runtime_error("Process::FreeMemory - Freeing memory failed.");
}
Process::operator HANDLE(void) const
{
	return m_handle;
}

HANDLE Process::GetHandle(void) const
{
	return m_handle;
}
DWORD Process::GetProcessIDByName(char const* processName)
{
	HANDLE snapshotHandle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	PROCESSENTRY32 entry = { 0 };
	entry.dwSize = sizeof(PROCESSENTRY32);

	bool found = false;
	found = Process32First(snapshotHandle, &entry);
	DWORD procID = 0;
	while (found)
	{
		if (strcmp(entry.szExeFile, processName) == 0)
		{
			procID = entry.th32ProcessID;
			break;
		}
		found = Process32Next(snapshotHandle, &entry);
	}
	CloseHandle(snapshotHandle);
	return procID;
}