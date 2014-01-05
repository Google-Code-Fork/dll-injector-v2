
#include "PEFile.h"
#include <iostream>


__declspec(naked) void TestStub()
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
		f.Infect(reinterpret_cast<char const*>(&TestStub), 6, 1);
		f.SaveAs("c:/FrequencyTester - modified.exe");
		std::cout << "File has been succesfuly infected! " << std::endl;
	}
	catch (std::runtime_error& e)
	{
		std::cout << e.what();
	}
	std::cin.get();
	return 0;
}

