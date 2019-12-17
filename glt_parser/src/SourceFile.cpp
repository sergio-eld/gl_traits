#include "SourceFile.h"

#include <cassert>

std::unique_ptr<ISourceFile> ISourceFile::Create(std::string&& filePath, Type type)
{
	switch (type)
	{
	case ISourceFile::text_source:
		return std::make_unique<SourceFile<text_source>>(std::move(filePath));
	case ISourceFile::header_common:
		return std::make_unique<SourceFile<header_common>>(std::move(filePath));
	case ISourceFile::header_shader:
		return std::make_unique<SourceFile<header_shader>>(std::move(filePath));
	case ISourceFile::unknown:
		return std::make_unique<SourceFile<unknown>>(std::move(filePath));
	default:
		assert(false && "Unknown Source file type");
		return std::unique_ptr<ISourceFile>();
	}

}
