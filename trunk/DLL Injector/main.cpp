#include <iostream>
#include "Injection.h"

int main(int argc, char* argv[])
{
	Injection inj("c:/DLLAPP.dll", "chrome.exe");
	try
	{
		inj.Inject();
		inj.Eject();
	}
	catch (std::runtime_error& e)
	{
		std::cout << "Error: " << e.what() << std::endl;
	}
	std::cin.get();
}