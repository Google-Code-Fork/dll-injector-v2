#include "Injection.h"
#include "Process.h"
Injection::Injection(char const* libraryName, char const* processName)
	:
	m_injected(false),
	m_libPath(libraryName),
	m_procName(processName)
{

}

Injection::~Injection(void)
{
	if (m_injected)
		Eject();
}

void Injection::Inject(void)
{
	if (GetFileAttributesA(m_libPath.c_str()) == INVALID_FILE_ATTRIBUTES)
		throw std::runtime_error("Injection::Inject - Library not found.");
	if (!Process::GetProcessIDByName(m_procName.c_str()))
		throw std::runtime_error("Injection::Inject - Process not running.");

	FARPROC loadLibAddress = GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");

	Process process(m_procName.c_str());
	LPVOID alloc = process.AllocMemory(m_libPath.size() + 1);
	process.WriteMemory(alloc, m_libPath.c_str(), m_libPath.size() + 1);
	m_moduleHandle = reinterpret_cast<HMODULE>(process.CallFunction(loadLibAddress, alloc));
	process.FreeMemory(alloc);
	m_injected = true;
}
void Injection::Eject(void)
{
	if (!m_injected)
		throw std::runtime_error("Injection::Eject - Library is not injected.");
	

	FARPROC freeLibAddress = GetProcAddress(GetModuleHandle("kernel32.dll"), "FreeLibrary");

	Process process(m_procName.c_str());
	process.CallFunction(freeLibAddress, m_moduleHandle);
	m_injected = false;
}
bool Injection::IsInjected(void) const
{
	return m_injected;
}

std::string const& Injection::GetLibraryPath(void) const
{
	return m_libPath;
}
std::string const& Injection::GetProcessName(void) const
{
	return m_procName;
}