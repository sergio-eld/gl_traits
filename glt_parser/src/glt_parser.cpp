
#include "IParser.h"

class ShaderParser
{
    std::string name_;
    std::vector<fsys::path> sources_;

public:
    ShaderParser(const std::string& fpath,
        const std::string& namePred, 
        const std::string& extensions = std::string())
        : name_(namePred),
        sources_(GetFilePaths(fpath, namePred, extensions))
    {}

private:

    static std::vector<fsys::path> GetFilePaths(const std::string& fpath,
        const std::string& namePred, const std::string& extensions)
    {
        fsys::path sourceDir{ fpath };
        if (!fsys::is_directory(sourceDir))
            sourceDir = sourceDir.parent_path();

        // change for assertions, source dir existance is validated by PathArgument
        if (!fsys::exists(sourceDir))
            throw std::exception("Source directory doesn't exist!");

        // check outside the constructor
        if (fsys::is_empty(sourceDir))
            throw std::exception("Source directory is empty!");

        // check outside
        if (namePred.empty())
            throw std::exception("Name Predicate is empty!");

        std::string ext{ R"((.txt))" };
        if (!extensions.empty())
        {
            std::regex regExt{ R"(.\w+)" };
            auto ext_begin = std::sregex_iterator(extensions.cbegin(), extensions.cend(), regExt);
            auto ext_end = std::sregex_iterator();

            for (std::sregex_iterator i = ext_begin; i != ext_end; ++i)
                ext += "|(" + i->str() + ")";

        }
        std::string patName{ namePred + R"(\w*)" + ext };
        std::regex regName{ patName };
        std::smatch sm;

        std::vector<fsys::path> out;

        for (auto& p : fsys::directory_iterator(sourceDir))
        {
            if (!p.is_regular_file())
                continue;

            std::string path = p.path().generic_string();
            if (std::regex_search(path, sm, regName))
                out.push_back(p.path());
        }


        return out;
    }

};

int main(int argc, const char** argv)
{
    /*
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
        */

    ShaderParser sh{ "C:/repos/gl_traits/glt_parser/sandbox", "shader", ".fs .vs" };


	ComLineParser cl_parser{ argc, argv };
    if (cl_parser.EmptyArgs())
    {
        cl_parser.PrintUsage();
        return -1;
    }

	if (!cl_parser.Success())
	{
		cl_parser.PrintErrors();
        cl_parser.PrintUsage();
		return -1;
	}



	return 0;
}

