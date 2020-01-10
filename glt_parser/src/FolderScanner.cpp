#include "FolderScanner.h"

#include <algorithm>
#include <regex>

#include <cassert>

template <ShaderFileInfo::ShaderType ... types>
struct source_types {};

using ExtensionsList = source_types<
	ShaderFileInfo::shader_vertex,
	ShaderFileInfo::shader_fragment,
	ShaderFileInfo::shader_geometry,
	ShaderFileInfo::shader_compute>;


struct FolderScanner::PrivateImpl
{
	template <ShaderFileInfo::ShaderType ... types>
	constexpr static std::set<ShaderFileInfo::ShaderExtensionInfo>
		init_extensions(source_types<types...>)
	{
		std::set<ShaderFileInfo::ShaderExtensionInfo> out;
		(out.emplace(ShaderFileInfo::ShaderExtensionInfo{ std::string(), types }), ...);
		return out;
	}

	using RefExt = std::reference_wrapper<ShaderFileInfo::ShaderExtensionInfo>;
	using CRefExt = std::reference_wrapper<const ShaderFileInfo::ShaderExtensionInfo>;

	template <class Iter>
	static std::optional<RefExt> FindType(ShaderFileInfo::ShaderType t, Iter first, Iter last)
	{
		Iter found = std::find_if(first, last,
			[&](const ShaderFileInfo::ShaderExtensionInfo& ext)
		{
			return ext.type == t;
		});

		if (found->type != t)
			return std::nullopt;

		return RefExt((ShaderFileInfo::ShaderExtensionInfo&)*found);
	}

	template <class Iter>
	static std::optional<RefExt> FindType(const std::string& strExtension, Iter first, Iter last)
	{
		Iter found = std::find_if(first, last,
			[&](const ShaderFileInfo::ShaderExtensionInfo& ext)
		{
			return ext.extension == strExtension;
		});

		if (found->extension != strExtension)
			return std::nullopt;

		return RefExt((ShaderFileInfo::ShaderExtensionInfo&)*found);
	}

};


FolderScanner::FolderScanner()
	: extensions_(PrivateImpl::init_extensions(ExtensionsList()))
{
	/*
	std::optional<PrivateImpl::RefExt> found =
		PrivateImpl::FindType(ShaderFileInfo::header_common, 
			extensions_.begin(), 
			extensions_.end());

	assert(found && "Failed to find extension type for header on FolderScanner initialization");

	ShaderFileInfo::ShaderExtensionInfo& eHeader = *found;
	eHeader.extension = ".h";*/
}


void FolderScanner::SetExtension(ShaderFileInfo::ShaderType t, std::string_view ext)
{
	std::optional<PrivateImpl::RefExt> found =
		PrivateImpl::FindType(t, extensions_.begin(), extensions_.end());

	if (!found)
		throw std::invalid_argument("FolderScanner::SetExtension::Invalid shader type!");

	ShaderFileInfo::ShaderExtensionInfo& extType = *found;
	extType.extension = ext;
}

bool FolderScanner::ExtensionAssigned(ShaderFileInfo::ShaderType t) const
{
	std::optional<PrivateImpl::RefExt> found =
		PrivateImpl::FindType(t, extensions_.begin(), extensions_.end());

	if (!found)
		throw std::invalid_argument("FolderScanner::SetExtension::Shader type not registered!");

	ShaderFileInfo::ShaderExtensionInfo& extType = *found;

	return !extType.extension.empty();
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

std::list<ShaderFileInfo> FolderScanner::SearchSources(const fsys::path& sourceFolder, 
	const std::set<ShaderFileInfo::ShaderExtensionInfo>& extensions)
{
	assert(fsys::exists(sourceFolder) && "Source folder does not exist!");

	if (extensions.empty())
		throw std::invalid_argument("FolderScanner::SearchSources() list of extensions is empty!");

	std::vector<PrivateImpl::CRefExt> exts;
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

		exts.push_back(e);
		extCollection += "(" + e.extension + "$)|";
	}
	
	if (invalidExtensions == extensions.size())
		throw std::invalid_argument("FolderScanner::SearchSources() all extensions are invalid!");

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

		out.push_back(ShaderFileInfo(std::move(path), exts[extTypeIndex]));
	}

	return out;
}