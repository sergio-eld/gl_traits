#include "ParseAlgorithm.h"

#include <iostream>
#include <string_view>

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
		std::vector<Variable2> vars =
			ParseAlgorithm<ISourceFile::text_source>::Parse(filePath);

		std::string_view type;
		for (const Variable2& v : vars)
		{
			using namespace std::literals;

			static std::string_view vertex_in = "vertex_in"sv,
				uniform = "uniform"sv,
				var_in = "var_in"sv,
				var_out = "var_out"sv;

			switch (v.type)
			{
			case Variable2::vertex_in:
				type = vertex_in;
				break;
			case Variable2::uniform:
				type = uniform;
				break;
			case Variable2::var_in:
				type = var_in;
				break;
			case Variable2::var_out:
				type = var_out;
				break;
			default:
				throw std::exception("Unhandled case");
				break;
			}

			std::cout << type << " " << v.location << " " << v.typeGLSL <<
				" " << v.name << std::endl;
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return -1;
	}
	return 0;
}