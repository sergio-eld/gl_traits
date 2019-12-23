#include "SourceFile.h"

#include <cassert>

std::unique_ptr<ISourceFile> ISourceFile::Create(ShaderFileInfo&& info)
{
	assert(info.shaderType != ShaderFileInfo::none);
	return std::make_unique<SourceFile<ShaderFileInfo::text_source>>(std::move(info.path),
		info.shaderType);
}

std::unique_ptr<ISourceFile> ISourceFile::Create(fsys::path&& filePath, 
	ShaderFileInfo::SourceType sourceType, ShaderFileInfo::ShaderType shaderType)
{
	switch (sourceType)
	{
	case ShaderFileInfo::text_source:
		assert(shaderType != ShaderFileInfo::none && "Invalid shader type!");
		return std::make_unique<SourceFile<ShaderFileInfo::text_source>>(std::move(filePath), shaderType);
	case ShaderFileInfo::header_common:
		return std::make_unique<SourceFile<ShaderFileInfo::header_common>>(std::move(filePath));
	default:
		assert(false && "Unknown Source file type");
		return std::unique_ptr<ISourceFile>();
	}

}
