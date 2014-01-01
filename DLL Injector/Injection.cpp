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

}

void Injection::Inject(void)
{
	FARPROC loadLibAddress = GetProcAddress(GetModuleHandle(m_libPath.c_str()), "LoadLibraryA");

	Process proc(m_procName.c_str());




	m_injected = true;
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