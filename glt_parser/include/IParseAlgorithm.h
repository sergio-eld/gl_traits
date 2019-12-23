#pragma once

#include "IDataType.h"

#include <vector>

struct IParseAlgorithm
{
	virtual ShaderFileInfo::SourceType SourceType() const = 0;

	virtual std::vector<Variable> Parse(const fsys::path& filePath) = 0;

	std::vector<Variable> operator()(const fsys::path& filePath)
	{
		return Parse(filePath);
	}

	static std::unique_ptr<IParseAlgorithm> GetAlgorithm(ShaderFileInfo::SourceType sourceType);
	//static std::unique_ptr<IParseAlgorithm> GetAlgorithm(ShaderFileInfo::ShaderType);

	~IParseAlgorithm() = default;
};