#pragma once

#include "ISourceFile.h"
#include "IVariable.h"

#include <vector>

template <ISourceFile::Type type>
class ParseAlgorithm
{

public:
	static std::vector<Variable2> Parse(const std::string& filePath);

};