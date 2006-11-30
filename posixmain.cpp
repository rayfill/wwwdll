#include "wwwdll.cpp"
#include <util/LexicalCast.hpp>
#include <iostream>

int usage()
{
	std::cout << "invalid command line option." << std::endl;
}

int main(int argc, char** argv)
{
	try
	{
		if (argc < 2)
			return usage();

		const int wakeupThreads = lexicalCast<int>(argv[argc-1]);

		

	}
	catch (std::exception& e)
	{
		std::cerr << "raise exception: " << e.what() << std::endl;
	}
	catch (...)
	{
		std::cerr << "unknown exception raised." << std::endl;
	}

	return 0;
}
