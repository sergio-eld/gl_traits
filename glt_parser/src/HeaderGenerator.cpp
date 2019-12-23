﻿#include "IHeaderGenerator.h"

#include <cassert>
#include <fstream>

bool IHeaderGenerator::compare_less(std::reference_wrapper<const Variable> w1, 
	std::reference_wrapper<const Variable> w2)
{
	const Variable &v1 = w1.get(),
		&v2 = w2.get();
	return v1 < v2;
}

IHeaderGenerator::IHeaderGenerator(fsys::path && outputFolder)
	: outputFolder_(std::move(outputFolder))
{
	assert(fsys::exists(outputFolder_) && "Output folder does not exist!");
}

void IHeaderGenerator::CollectVariables(const ISourceFile & sf)
{
	size_t vars = sf.VarsCount();
	for (size_t i = 0; i != vars; ++i)
		vars_.emplace(sf.GetVariable(i));
}

void IHeaderGenerator::GenerateCommonHeader()
{
	fsys::path commonPath{ outputFolder_ };
	commonPath.append(commonName);

	std::fstream commonHeader{ commonPath, std::fstream::out };

	if (!commonHeader.is_open())
	{
		std::string msg{ "Failed to open file " + commonPath.generic_string() };
		throw std::exception(msg.c_str());
	}

	commonHeader.clear();

    commonHeader << "#pragma once\n\n#include \"glm/glm.hpp\"\n\n"
        "#define GLSLT_TYPE(NAME, VAR_NAME, TYPE) struct name{\\\n"
        "constexpr static const char* name(){ return \"VAR_NAME\";}\\\n"
        "using type = TYPE};\n\n";
    // TODO: write generation time, user, files and\or license, etc.

    for (const Variable& var : vars_)
    {
        commonHeader << "GLSLT_TYPE(" << var.name << '_' << var.typeGLSL << ", " <<
            var.name << ", " << Variable::cpp_glsl_type(var.glslDataType, var.typeGLSL) << ")\n";
    }

}

size_t IHeaderGenerator::VarsCollected() const
{
	return vars_.size();
}

IHeaderGenerator::Container::const_iterator IHeaderGenerator::cbegin() const
{
	return vars_.cbegin();
}

IHeaderGenerator::Container::const_iterator IHeaderGenerator::cend() const
{
	return vars_.cend();
}