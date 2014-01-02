#include "Process.h"
#include <TlHelp32.h>
Process::Process(char const* processName)
	:
	m_handle(INVALID_HANDLE_VALUE)
{
	DWORD procID = Process::GetProcessIDByName(processName);
	m_handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procID);

	if (m_handle == NULL)
		throw std::runtime_error("Process::Process - Could not open the process");
}
Process::Process(Process& rhs)
	:
	m_handle(INVALID_HANDLE_VALUE),
	m_allocs(rhs.m_allocs)
{
	DWORD procID = GetProcessId(rhs.m_handle);
	m_handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procID);
	rhs.m_allocs.clear(); // We transfer the mem allocs to the new object
}
Process::~Process(void)
{
	std::vector<LPVOID>::iterator iter = m_allocs.begin();
	while (iter != m_allocs.end())
	{
		FreeMemory(*iter);
		iter = m_allocs.begin();
	}

	if (m_handle != INVALID_HANDLE_VALUE && m_handle != NULL)
		CloseHandle(m_handle);
}
Process& Process::operator=(Process& rhs)
{
	DWORD procID = GetProcessId(rhs.m_handle);
	m_handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procID);
	m_allocs = rhs.m_allocs;
	
	rhs.m_allocs.clear();  // We transfer the mem allocs to the new object
	return *this;
}
Process::operator HANDLE(void) const
{
	return m_handle;
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
	m_allocs.erase(position);

	bool success = VirtualFreeEx(m_handle, address, 0, MEM_RELEASE);

	if (!success)
		throw std::runtime_error("Process::FreeMemory - Freeing memory failed.");
}
void Process::WriteMemory(LPVOID address, LPCVOID data, size_t size) const
{
	SIZE_T bytesWritten = 0;
	bool success = WriteProcessMemory(m_handle, address, data, size, &bytesWritten);
	if (!success || bytesWritten != size)
		throw std::runtime_error("Process::WriteMemory - Writing memory failed.");
}
void Process::ReadMemory(LPCVOID address, LPVOID buffer, size_t size) const
{
	SIZE_T bytesRead = 0;
	bool success = ReadProcessMemory(m_handle, address, buffer, size, &bytesRead);
	if (!success || bytesRead != size)
		throw std::runtime_error("Process::ReadMemory - Reading memory failed.");
}
DWORD Process::CallFunction(LPCVOID address, LPVOID arg)
{
	HANDLE thread = CreateRemoteThread(m_handle, 0, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(address), arg, 0, 0);
	if (!thread)
		throw std::runtime_error("Process::CallFunction - Error creating a thread.");
	DWORD threadState = WaitForSingleObject(thread, 5000);
	if (threadState != WAIT_OBJECT_0)
		throw std::runtime_error("Process::CallFunction - Error executing the thread function - timed out.");

	DWORD exitCode = 0;
	GetExitCodeThread(thread, &exitCode);
	if (!exitCode)
		throw std::runtime_error("Process::CallFunction - Error executing the thread function - exit code equals 0.");
	return exitCode; 
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