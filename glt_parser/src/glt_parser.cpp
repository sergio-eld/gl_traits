
#include "IParser.h"
#include "IGeneratorGLT.h"

#include <cassert>

#include <regex>

#include "FolderScanner.h"
#include "SourceFile.h"
#include "IHeaderGenerator.h"


int main(int argc, const char** argv)
{
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

	std::optional<rIArgument> foundArgSourcePath = IArgument::Find("-s"),
		foundArgOutPath = IArgument::Find("-d"),
		foundArgNamePred = IArgument::Find("-p"),
		foundArgExtensions = IArgument::Find("-e");

	assert(foundArgSourcePath.has_value() && "Failed to find Source Path argument!");
	assert(foundArgOutPath.has_value() && "Failed to find Output Path argument!");
	assert(foundArgNamePred.has_value() && "Failed to find Name Predicates argument!");
	assert(foundArgExtensions.has_value() && "Failed to find File extensions argument!");

	IArgument &argSourcePath = *foundArgSourcePath,
		&argOutPath = *foundArgOutPath,
		&argNamePred = *foundArgNamePred,
		&argExtensions = *foundArgExtensions;

	
	fsys::path sourcePath{ argSourcePath.Value() },
		outputPath{ argOutPath.Value() };

	if (!fsys::exists(sourcePath))
	{
		std::cerr << "Source path does not exist!" << std::endl;
		return -1;
	}

	if (!fsys::exists(outputPath))
	{
		std::cerr << "Output path does not exist!" << std::endl;
		return -1;
	}

	// TODO: all received cl arguments must be valid and accessable by this point!

	FolderScanner fs{};

    ///////////////////////////////////////////////////////
    // This is a workaround.
    // TODO: 
    // 1. Get a list of cl arguments that provide extension types
    // 2. loop: fs.SetExtension(std::any_cast<ShaderFileInfo::ShaderType>(arg.get_value()));
	std::regex extPat{ R"(.\w+)" };

	const std::string& args = argExtensions.Value();

	std::sregex_iterator start{ args.cbegin(), args.cend(), extPat },
		end{};

	auto extIter = ShaderFileInfo::list_types.cbegin();

	while (start != end)
		fs.SetExtension(*extIter++, (start++)->str());
    //////////////////////////////////////////////////////

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

    // if verbose
    std::cout << "Generated files have been written to " << argOutPath.Value() << std::endl;

	return 0;
}

