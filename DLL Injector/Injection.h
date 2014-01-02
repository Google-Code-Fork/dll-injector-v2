#ifndef INJECTION_H
#define INJECTION_H

#include <string>
#include <Windows.h>
class Injection
{
public:
	Injection(char const* libraryPath, char const* processName);
	Injection(Injection const& rhs) = delete;
	~Injection(void);

	Injection& operator=(Injection const& rhs) = delete;

	void Inject(void);
	void Eject(void);

	bool IsInjected(void) const;
	std::string const& GetLibraryPath(void) const;
	std::string const& GetProcessName(void) const;

private:
	bool		m_injected;

	std::string	m_libPath;
	std::string m_procName;

	HMODULE		m_moduleHandle;
};
#endif