#include "ParseAlgorithm/ParseAlgorithm.h"

#include <iostream>
#include <string_view>

int main(int argc, const char* argv[])
{
	if (argc != 2)
	{
		std::cout << "Usage: ./*.exe \"path_to_shader_file\"" << std::endl;
		return -1;
	}
	fsys::path filePath{ argv[1] };

	try
	{
		std::vector<Variable> vars =
			ParseAlgorithm<ShaderFileInfo::text_source>::ParseImpl(filePath);

		std::string_view type;
		for (const Variable& v : vars)
		{
			using namespace std::literals;

			static std::string_view vertex_in = "vertex_in"sv,
				uniform = "uniform"sv,
				var_in = "var_in"sv,
				var_out = "var_out"sv;

			switch (v.type)
			{
			case Variable::vertex_in:
				type = vertex_in;
				break;
			case Variable::uniform:
				type = uniform;
				break;
			case Variable::var_in:
				type = var_in;
				break;
			case Variable::var_out:
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