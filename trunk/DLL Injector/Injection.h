#ifndef INJECTION_H
#define INJECTION_H

#include <string>
class Injection
{
public:
	Injection(char const* libraryPath, char const* processName);

	~Injection(void);

	void Inject(void);
	void Free(void);

	bool IsInjected(void) const;
	std::string const& GetLibraryPath(void) const;
	std::string const& GetProcessName(void) const;

private:
	bool		m_injected;

	std::string	m_libPath;
	std::string m_procName;
};
#endif