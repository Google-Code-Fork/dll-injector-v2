#include <iostream>
#include "Injection.h"
#include "Process.h"
#include "PEFile.h"

__declspec(naked) void Stub()
{
	__asm
	{
		push 0x12345678
		ret
	}
}

int main(int argc, char* argv[])
{
	try

	{
		PEFile f("c:/FrequencyTester.exe");

		f.Infect(reinterpret_cast<char*>(&Stub), 6, 0, 1);
		f.SaveAs("c:/FrequencyTester - modified.exe");
	}
	catch (std::runtime_error& e)
	{
		std::cout << e.what();
		std::cin.get();
	}
	return 0;
	char const* processName = NULL;
	char const* libraryPath = NULL;

	for (int i = 0; i < argc; i++)
	{
		if ( (strcmp(argv[i], "-p") == 0) && (i + 1 < argc) )
			processName = argv[i + 1];
		else if ( (strcmp(argv[i], "-l") == 0) && (i + 1 < argc) )
			libraryPath = argv[i + 1];
	}

	if (processName && libraryPath)
	{
		try
		{
			Injection inj(libraryPath, processName);

			inj.Inject();
			inj.Eject();
		}
		catch (std::runtime_error& e)
		{
			std::cout << "Error: " << e.what() << std::endl;
		}
	}
	else
		std::cout << "Please specify both, process name and library path." << std::endl
				  << "Possible args:   -p [processName]   -l [libraryPath]";
	std::cin.get();
}