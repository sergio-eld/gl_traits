#pragma once

#include "IParseAlgorithm.h"


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

	static std::vector<Variable> ParseImpl(const fsys::path& filePath);

};

