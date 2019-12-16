#include "ParseAlgorithm.h"

#include <iostream>

int main(int argc, const char* argv[])
{
	if (argc != 2)
	{
		std::cout << "Usage: ./*.exe \"path_to_shader_file\"" << std::endl;
		return -1;
	}
	std::string filePath{ argv[1] };

	try
	{
		std::vector<std::unique_ptr<IVariable>> vars =
			ParseAlgorithm<ISourceFile::text_source>::Parse(filePath);

		for (const std::unique_ptr<IVariable>& v : vars)
			std::cout << v->Type() << " " << v->location() << " " << v->TypeGLSL() <<
			" " << v->Name() << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return -1;
	}
	return 0;
}