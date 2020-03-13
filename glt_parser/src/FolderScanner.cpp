#include "FolderScanner.h"

#include <algorithm>
#include <regex>

#include <cassert>

FolderScanner::FolderScanner()
{

}


void FolderScanner::SetExtension(ShaderFileInfo::ShaderType t, std::string_view ext)
{
    extensions_.emplace(ShaderFileInfo::ShaderExtensionInfo{ std::string(ext), t });

}


std::optional<ShaderFileInfo> FolderScanner::FetchSourceFile()
{
	if (locatedSources_.empty())
		return std::nullopt;

	ShaderFileInfo source = std::move(*locatedSources_.begin());
	locatedSources_.erase(locatedSources_.begin());

	return source;
}

void FolderScanner::SearchSources(const fsys::path& sourceFolder)
{
	try
	{
		std::list<ShaderFileInfo> located = SearchSources(sourceFolder, extensions_);
		std::move(located.begin(), located.end(), std::back_inserter(locatedSources_));
	}
	catch (const std::invalid_argument& e)
	{
		throw e;
	}
}

void FolderScanner::operator()(std::string_view folderPath)
{
	return SearchSources(folderPath);
}

#pragma message("FolderScanner::SearchSources() does not support searching for .h sources (CommonTypes)")
std::list<ShaderFileInfo> FolderScanner::SearchSources(const fsys::path& sourceFolder, 
	const std::set<ShaderFileInfo::ShaderExtensionInfo>& extensions)
{
	assert(fsys::exists(sourceFolder) && "Source folder does not exist!");

	if (extensions.empty())
		throw std::invalid_argument("FolderScanner::SearchSources() list of extensions is empty!");

    std::vector<ShaderFileInfo::ShaderType> exts;

	std::string extCollection;

	int invalidExtensions = 0;

	for (const ShaderFileInfo::ShaderExtensionInfo& e : extensions)
	{
		if (e.extension.empty() ||
			e.extension[0] != '.')
		{
			++invalidExtensions;
			continue;
		}

		exts.push_back(e.type);
		extCollection += "(" + e.extension + "$)|";
	}
	
	if (invalidExtensions == extensions.size())
		throw std::invalid_argument("FolderScanner::SearchSources() all extensions are invalid!");

    // remove last '|' char
	extCollection.resize(extCollection.size() - 1);

	std::regex extPattern{ extCollection };
	std::smatch sm;

	std::list<ShaderFileInfo> out;

	for (auto& p : fsys::directory_iterator(sourceFolder))
	{
		if (!p.is_regular_file())
			continue;

		fsys::path path = std::move(p);
		const std::string& strpath = path.generic_string();
		if (!std::regex_search(strpath, sm, extPattern))
			continue;

		int extTypeIndex = 0;
		auto sub = ++sm.cbegin();
		while (sub != sm.cend() && !(sub++)->matched)
			++extTypeIndex;

		assert(extTypeIndex != exts.size() && "Regex extension index is out of range!");

       
		out.push_back(ShaderFileInfo(std::move(path), exts[extTypeIndex],
            ShaderFileInfo::text_source));
	}

	return out;
}