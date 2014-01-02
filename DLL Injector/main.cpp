#include <iostream>
#include "Injection.h"
#include "Process.h"
int main(int argc, char* argv[])
{
	std::cerr << "NIGGER";
	std::cin.get();
	try
	{
		Injection inj("c:/DLLAPP.dll", "chrome.exe");

		inj.Inject();
		inj.Eject();
	}
	catch (std::runtime_error& e)
	{
		std::cout << "Error: " << e.what() << std::endl;
	}
	std::cin.get();
}