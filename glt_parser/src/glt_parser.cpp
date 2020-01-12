
#include "IParser.h"
#include "IGeneratorGLT.h"

#include <cassert>

#include <regex>

#include "FolderScanner.h"
#include "SourceFile.h"
#include "IHeaderGenerator.h"

IArgument& AssertArg(std::optional<RefIArgument>&& rarg)
{
    assert(rarg.has_value() && "Ergument is missing!");
    return *rarg;
}

int main(int argc, const char** argv)
{   
	ComLineParser& parser = *ComLineParser::parser;

	if (!parser(argc, argv))
	{
		parser.PrintErrors();
		parser.PrintUsage();
		return -1;
	}

	// TODO: all received cl arguments must be valid and accessable by this point!

	FolderScanner fs{};

    std::vector<std::string> shaderTags{
        "--vert",
        "--frag",
        "--geom",
        "--comp"
    };

    for (const std::string& shTag : shaderTags)
    {
        std::pair<ShaderFileInfo::ShaderType, const std::set<std::string_view>&>
            extensions = AssertArg(parser.GetArgument(shTag));

        for (std::string_view ext : extensions.second)
            fs.SetExtension(extensions.first, ext);
    }


    fsys::path sourcePath = AssertArg(parser.GetArgument("-s")),
        outputPath = AssertArg(parser.GetArgument("-d"));

	try
	{
		fs.SearchSources(sourcePath);
	}
	catch (const std::invalid_argument& e)
	{
		std::cout << e.what() << std::endl;
		return -1;
	}

	std::vector<std::unique_ptr<ISourceFile>> sources;



	std::optional<ShaderFileInfo> fInfo = fs.FetchSourceFile();
	while (fInfo)
	{
		ShaderFileInfo& info = *fInfo;
		sources.emplace_back(ISourceFile::Create(std::move(info)));
		fInfo = fs.FetchSourceFile();
	}

	IHeaderGenerator generator{ std::move(outputPath) };

	for (const std::unique_ptr<ISourceFile>& sf : sources)
		generator.AppendSource(*sf);


	try 
	{
		generator.GenerateCommonHeader();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return -1;
	}

	return 0;
}

