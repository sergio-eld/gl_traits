﻿
#include "IParser.h"
#include "IGeneratorGLT.h"

#include <cassert>

#include <regex>

#include "FolderScanner.h"
#include "SourceFile.h"
#include "IHeaderGenerator.h"


int main(int argc, const char** argv)
{
	ComLineParser& parser = *ComLineParser::parser;


	if (!parser(argc, argv))
	{
		parser.PrintErrors();
		parser.PrintUsage();
		return -1;
	}

	ComLineParser cl_parser{ argc, argv };

	/*
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
	}*/

	std::optional<rIArgumentOld> foundArgSourcePath = IArgumentOld::Find("-s"),
		foundArgOutPath = IArgumentOld::Find("-d"),
		foundArgNamePred = IArgumentOld::Find("-p"),
		foundArgExtensions = IArgumentOld::Find("-e");

	assert(foundArgSourcePath.has_value() && "Failed to find Source Path argument!");
	assert(foundArgOutPath.has_value() && "Failed to find Output Path argument!");
	assert(foundArgNamePred.has_value() && "Failed to find Name Predicates argument!");
	assert(foundArgExtensions.has_value() && "Failed to find File extensions argument!");

	IArgumentOld &argSourcePath = *foundArgSourcePath,
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

	std::regex extPat{ R"(.\w+)" };

	const std::string& args = argExtensions.Value();

	std::sregex_iterator start{ args.cbegin(), args.cend(), extPat },
		end{};

	auto extIter = ShaderFileInfo::list_types.cbegin();

	while (start != end)
		fs.SetExtension(*extIter++, (start++)->str());

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

