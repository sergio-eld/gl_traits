#pragma once

#include "IDataType.h"

#include <string_view>
#include <set>
#include <list>

#include <optional>

namespace fsys = std::filesystem;

class FolderScanner
{
	std::set<ShaderFileInfo::ShaderExtensionInfo> extensions_;
	std::list<ShaderFileInfo> locatedSources_;

public:

	FolderScanner();

	void SetExtension(ShaderFileInfo::ShaderType t, std::string_view ext);

	//bool ExtensionAssigned(ShaderFileInfo::ShaderType t) const;

	std::optional<ShaderFileInfo> FetchSourceFile();

	void SearchSources(const fsys::path& sourceFolder);
	void operator()(std::string_view folderPath);


	static std::list<ShaderFileInfo> SearchSources(const fsys::path& sourceFolder,
		const std::set<ShaderFileInfo::ShaderExtensionInfo>& extensions);

private:

    using RefExt = std::reference_wrapper<ShaderFileInfo::ShaderExtensionInfo>;
    using CRefExt = std::reference_wrapper<const ShaderFileInfo::ShaderExtensionInfo>;



};
