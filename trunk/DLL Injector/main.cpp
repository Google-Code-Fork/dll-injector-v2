#include <iostream>
#include "Injection.h"
#include "Process.h"


int main(int argc, char* argv[])
{
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
			std::cout << "Library has been succesfuly injected into the process" << std::endl
				      << "Press any key to eject the library..." << std::endl;
			std::cin.get();
			inj.Eject();
			std::cout << "Library has been ejected" << std::endl;
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