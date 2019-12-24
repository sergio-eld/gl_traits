#pragma once

#include "IDataType.h"

struct ISourceFile
{
	virtual const fsys::path& Name() const = 0;
	virtual ShaderFileInfo::SourceType SourceType() const = 0;
	virtual ShaderFileInfo::ShaderType ShaderType() const = 0;
	//virtual void Parse() = 0;

	virtual ~ISourceFile() = default;

	static std::unique_ptr<ISourceFile> Create(ShaderFileInfo&& info);
	static std::unique_ptr<ISourceFile> Create(fsys::path&& filePath, 
		ShaderFileInfo::SourceType, ShaderFileInfo::ShaderType = ShaderFileInfo::none);
	
	// iterators
	virtual size_t VarsCount() const = 0;
	virtual const Variable& GetVariable(size_t indx) const = 0;

};
