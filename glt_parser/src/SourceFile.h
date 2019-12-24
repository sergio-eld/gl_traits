#pragma once

#include "ISourceFile.h"
#include "IVariable.h"

#include "IParseAlgorithm.h"

#include <vector>
#include <string_view>

#include <filesystem>

namespace fsys = std::filesystem;

template <ShaderFileInfo::SourceType type>
class SourceFile : public ISourceFile
{
	fsys::path filePath_;
	std::vector<Variable> variables_;
	ShaderFileInfo::ShaderType shaderType_;

	static inline std::unique_ptr<IParseAlgorithm> parseAlgorithm_ = 
		IParseAlgorithm::GetAlgorithm(type);

public:

	SourceFile(fsys::path&& filePath, ShaderFileInfo::ShaderType shType = ShaderFileInfo::none)
		: filePath_(std::move(filePath)),
		variables_((*parseAlgorithm_)(filePath_)),
		shaderType_(shType)
	{}

	const fsys::path& Name() const override
	{
		return filePath_;
	}

	ShaderFileInfo::SourceType SourceType() const override
	{
		return type;
	}

	ShaderFileInfo::ShaderType ShaderType() const override
	{
		return shaderType_;
	}


	size_t VarsCount() const override
	{
		return variables_.size();
	}

	const Variable& GetVariable(size_t indx) const override
	{
		if (indx >= VarsCount())
		{
			std::string msg = "Variable index is out of range! " + std::to_string(indx) +
				'/' + std::to_string(VarsCount());
			throw std::range_error(msg);
		}

		return variables_[indx];
	}

};


