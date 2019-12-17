#pragma once

#include "ISourceFile.h"

#include <string_view>
#include <string>
#include <set>
#include <list>

#include <filesystem>

namespace fsys = std::filesystem;

class FolderScanner
{
	friend struct PrivateImpl;
	struct PrivateImpl;

	using ExtensionType = std::pair<std::string, ISourceFile::Type>;

	static bool type_compare_less(const ExtensionType& e1, const ExtensionType& e2);
	using comparator_less = std::integral_constant<decltype(&type_compare_less),
		&type_compare_less>;

	std::set<ExtensionType, comparator_less> extensions_;
	std::list<fsys::path> locatedSources_;

public:

	FolderScanner();

	void SetExtension(ISourceFile::Type t, std::string_view ext);

	bool ExtensionAssigned(ISourceFile::Type t);

	std::unique_ptr<ISourceFile> FetchSourceFile();

};
