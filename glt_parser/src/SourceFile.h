﻿#pragma once

#include "ISourceFile.h"
#include "IVariable.h"

#include "ParseAlgorithm.h"

#include <vector>
#include <string_view>

#include <filesystem>

namespace fsys = std::filesystem;

template <ISourceFile::Type type>
class SourceFile : public ISourceFile
{
	std::string filePath_;
	std::vector<Variable2> variables_;

public:

	SourceFile(std::string&& filePath)
		: filePath_(std::move(filePath)),
		variables_(ParseAlgorithm<type>::Parse(filePath_))
	{}

	const std::string& Name() const override
	{
		return filePath_;
	}

	Type FileType() const override
	{
		return type;
	}
};


