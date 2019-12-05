#include "IGeneratorGLT.h"

#include "IShaderParser.h"

#include <string_view>
#include <vector>

#include <set>
#include <cassert>
#include <filesystem>

#include <iostream>

#include <thread>

namespace fsys = std::filesystem;

using ptrIShaderParser = std::unique_ptr<IShaderParser>;


class GeneratorGLT : public IGeneratorGLT
{

	static bool compLess(const ptrIShaderParser& sh1,
		const ptrIShaderParser& sh2)
	{
		return sh1->ProgName() < sh2->ProgName();
	}

	std::set<ptrIShaderParser,
		std::integral_constant<decltype(&GeneratorGLT::compLess),
		&GeneratorGLT::compLess>> progParsers_;


public:

	GeneratorGLT(const std::string& sourcePath,
		const std::string& outputPath,
		const std::string& namePredicates, const std::string& extensions,
		Severity sev = find_one)
	{
		assert(fsys::exists(sourcePath) && "Source path does not exist!");
		assert(fsys::exists(outputPath) && "Output path does not exist!");
		assert(!namePredicates.empty() && "Name predicates are empty!");

		std::vector<std::string_view> names = GetNames(namePredicates);
		
		// TODO: do not add empty parsers?
		for (std::string_view pred : names)
			progParsers_.emplace(IShaderParser::Create(sourcePath,
				std::string(pred.data(), pred.size()), extensions));

		if (progParsers_.empty() && sev != Severity::find_none)
			throw std::exception("Error: no shader source files found!");



		// TODO: format message to look compact - create newlines after each N chars
		std::string msg{"Failed to find shader source files with extensions [" + 
			extensions  + "] in \"" + sourcePath + "\""
			" for the following shader programs:\n"};

		for (const ptrIShaderParser& p : progParsers_)
			if (!p->SourceFilesCount())
				msg += p->ProgName() + '\n';

		switch (sev)
		{
		case IGeneratorGLT::find_one:
			std::cerr << "Warning: " << msg << std::endl;
			break;
		case IGeneratorGLT::find_none:
			std::cerr << "Warning: " << msg << std::endl;
			return;
		case IGeneratorGLT::find_all:
		{
			msg = "Error: " + msg;
			throw std::exception(msg.data());
		}
		default:
			assert(false && "Unhandled case!");
			break;
		}

		for (const ptrIShaderParser& p : progParsers_)
			p->ParseAll();

	}


	static std::vector<std::string_view> GetNames(const std::string& predicates)
	{
		std::vector<std::string_view> vec_names;

		size_t begin = 0;

		while (begin < predicates.size())
		{
			size_t nextPos = predicates.find(' ', begin);
			if (nextPos == std::string::npos)
				nextPos = predicates.size();

			if (nextPos - begin > 1)
				vec_names.emplace_back(std::next(predicates.c_str(), begin), 
					nextPos - begin);

			begin = nextPos + 1;
		}

		return vec_names;
	}

};

std::unique_ptr<IGeneratorGLT> IGeneratorGLT::Create(const std::string & sourcePath,
	const std::string& outputPath,
	const std::string & namePredicates, 
	const std::string & extensions, 
	Severity sev)
{
	return std::make_unique<GeneratorGLT>(sourcePath, outputPath, namePredicates,
		extensions, sev);
}