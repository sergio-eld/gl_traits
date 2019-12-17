#include "FolderScanner.h"

#include <algorithm>
#include <optional>


template <ISourceFile::Type ... types>
struct source_types {};

using SourceTypes = source_types<
	ISourceFile::text_source,
	ISourceFile::text_vertex_shader_source,
	ISourceFile::text_fragment_shader_source,
	ISourceFile::text_geometry_shader_source,
	ISourceFile::header_common,
	ISourceFile::header_shader>;


struct FolderScanner::PrivateImpl
{
	template <ISourceFile::Type ... types>
	constexpr static std::set<ExtensionType, comparator_less>
		init_extensions(source_types<types...>)
	{
		std::set<ExtensionType, comparator_less> out;
		(out.emplace(ExtensionType(std::string(), types)), ...);
		return out;
	}

	using Iter =
		std::set<ExtensionType, comparator_less>::iterator;
	using CIter = std::set<ExtensionType, comparator_less>::const_iterator;

	using RefExt = std::reference_wrapper<ExtensionType>;

	static std::optional<RefExt> FindType(ISourceFile::Type t, Iter first, Iter last)
	{
		Iter found = std::find_if(first, last,
			[&](const ExtensionType& ext)
		{
			return ext.second == t;
		});

		if (found->second != t)
			return std::nullopt;

		return RefExt((ExtensionType&)*found);
	}

	static std::optional<RefExt> FindType(ISourceFile::Type t, CIter first, CIter last)
	{
		CIter found = std::find_if(first, last,
			[&](const ExtensionType& ext)
		{
			return ext.second == t;
		});

		if (found->second != t)
			return std::nullopt;

		return RefExt((ExtensionType&)*found);
	}
};


bool FolderScanner::type_compare_less(const ExtensionType & e1, const ExtensionType & e2)
{
	return e1.first < e2.first ||
		e1.second < e2.second;
}

FolderScanner::FolderScanner()
	: extensions_(PrivateImpl::init_extensions(SourceTypes()))
{
}

void FolderScanner::SetExtension(ISourceFile::Type t, std::string_view ext)
{
	std::optional<PrivateImpl::RefExt> found =
		PrivateImpl::FindType(t, extensions_.begin(), extensions_.end());

	if (!found)
		throw std::invalid_argument("FolderScanner::SetExtension::Invalid shader type!");

	ExtensionType& extType = *found;
	extType.first = ext;
}

bool FolderScanner::ExtensionAssigned(ISourceFile::Type t)
{
	std::optional<PrivateImpl::RefExt> found =
		PrivateImpl::FindType(t, extensions_.begin(), extensions_.end());

	if (!found)
		throw std::invalid_argument("FolderScanner::SetExtension::Invalid shader type!");

	ExtensionType& extType = *found;

	return !extType.first.empty();
}

std::unique_ptr<ISourceFile> FolderScanner::FetchSourceFile()
{
	fsys::path sourcePath = std::move(*locatedSources_.begin());
	locatedSources_.erase(locatedSources_.begin());

	//auto found = extensions_.find(sourcePath.extension().generic_string());

	ISourceFile::Type type = extensions_.find()
	return ISourceFile::Create(sourcePath.generic_string(), )
}
