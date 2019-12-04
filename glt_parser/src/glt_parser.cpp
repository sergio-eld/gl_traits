
#include "IParser.h"


int main(int argc, const char** argv)
{
	const auto& args = IArgument::defaultArgs;

	std::string patParam{ R"(-\D)" },
		patPath{ R"(((?:[a-zA-Z]:)|.*)?([\\|/][^\\|/]+))" };

	std::regex regPath{ patPath };

	auto mk = regPath.mark_count();
	std::smatch sm;

	std::vector<std::string> paths
	{
		{ "C:\\" },
	{"C:/"},
	{"/"},
	{"./"},
	{"\\"},
	{"C:/Documents\\path"},
	{"C:\\Documents/path/"},
	{"C:\\Documents/  path stupid spaces /"},
	{"...\\Documents/  path stupid spaces /"}

	};

	for (const auto& p : paths)
	{
		static size_t count = 0;
		if (!std::regex_match(p, sm, regPath))
			std::cout << count << " failed: " << p << std::endl;
		++count;
	}

	std::string path{ "C:/path/path" },
		arg{ "-S" };

	std::regex pathOrArg{ R"((-\D)|((\D:)?[\|/]\w*))" };
	std::smatch smArg,
		smPath;

	bool matchedArg = std::regex_search(arg, smArg, pathOrArg),
		mathcedPath = std::regex_search(path, smPath, pathOrArg);

	ComLineParser cl_parser{ argc, argv };

	if (!cl_parser.Success())
	{
		cl_parser.PrintErrors();
		return -1;
	}


	//ComLineParser cl_parser{ argc, argv };


	return 0;
}

