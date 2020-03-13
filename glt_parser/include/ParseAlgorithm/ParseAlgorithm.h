#pragma once

#include "IParseAlgorithm.h"

// TODO: udate implementation according to https://www.khronos.org/opengl/wiki/Layout_Qualifier_(GLSL)

template <ShaderFileInfo::SourceType type>
class ParseAlgorithm : public IParseAlgorithm
{

public:

	ShaderFileInfo::SourceType SourceType() const override
	{
		return type;
	}

	std::vector<Variable> Parse(const fsys::path& filePath) override
	{
		return ParseImpl(filePath);
	}

	std::vector<Variable> Parse(std::string_view shaderSource) override
	{
		return ParseImpl(shaderSource);
	}

	static std::vector<Variable> ParseImpl(const fsys::path& filePath);
	static std::vector<Variable> ParseImpl(std::string_view shaderSource);
};

using ShaderSourceParceAlg = ParseAlgorithm<ShaderFileInfo::text_source>;