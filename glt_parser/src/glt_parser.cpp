
#include "IParser.h"
#include "IGeneratorGLT.h"

#include <cassert>


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

	try 
	{
		// TODO: add argument for severity (find none || find one || find all)
		std::unique_ptr<IGeneratorGLT> gltGenerator =
			IGeneratorGLT::Create(argSourcePath.Value(),
				argOutPath.Value(),
				argNamePred.Value(),
				argExtensions.Value());

	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return -1;
	}

	return 0;
}

