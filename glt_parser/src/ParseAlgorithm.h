#pragma once

#include "ISourceFile.h"
#include "IVariable.h"

#include <vector>

template <ISourceFile::Type type>
class ParseAlgorithm
{

public:
	static std::vector<std::unique_ptr<IVariable>> Parse(const std::string& filePath);

};