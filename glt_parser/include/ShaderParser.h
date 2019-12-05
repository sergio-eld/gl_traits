#pragma once

#include <string>

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
