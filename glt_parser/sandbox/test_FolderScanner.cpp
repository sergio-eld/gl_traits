#include "FolderScanner.h"

#include <iostream>
#include <regex>

// TODO: add ParseCL here to get CL arguments

int main(int argc, const char *argv[])
{
	if (argc != 3)	
	{
		std::cout << "Usage: ./*.exe \"source folder path\" -e \".ext1 .ext2 .ext3 ... .extN\"" << std::endl;
		return -1;
	}

	// get list of extensions
	

	static std::regex pat{ R"(.\w{1,30}(?=[\s|,|\0|\n]))" };
	std::string_view listExtensions{ argv[2] };

	std::regex_iterator<std::string_view::const_iterator>
		start{ listExtensions.cbegin(), listExtensions.cend(), pat },
		end{};

	try 
	{
		FolderScanner fs{};

		static std::vector<ShaderFileInfo::ShaderType> extTypes{
			ShaderFileInfo::shader_vertex,
		ShaderFileInfo::shader_fragment,
		ShaderFileInfo::shader_geometry,
		ShaderFileInfo::shader_compute };

		auto iterType = extTypes.cbegin();

		// TODO: check for valid extension
		while (start != end)
		{
			const std::match_results<std::string_view::const_iterator>& sm = *start;
			fs.SetExtension(*iterType++, sm.cbegin()->str());
			++start;
		}

		fs(std::string_view(argv[1]));

		std::optional<ShaderFileInfo> fileInfo = fs.FetchSourceFile();

		while (fileInfo != std::nullopt)
		{
			std::cout << fileInfo->shaderType << " " << fileInfo->path << std::endl;
			fileInfo = fs.FetchSourceFile();
		}

	}
	catch (const std::invalid_argument& a)
	{
		std::cout << "Error: " << a.what() << std::endl;
		return -1;
	}
	



	return 0;
}

