#include "..\include\IDataType.h"
#include "IDataType.h"

#include <cassert>

ShaderFileInfo::ShaderFileInfo(fsys::path && vpath, const ShaderExtensionInfo& vtype)
	: path(std::move(vpath)),
	shaderType(vtype.type),
	sourceType(text_source)
{
	assert(path.extension().generic_string() == vtype.extension &&
		"ShaderFileInfo::ShaderFileInfo() Extension names mismatch!");
}

ShaderFileInfo::ShaderFileInfo(fsys::path && vpath, 
	ShaderType shaderType, 
	SourceType sourceType)
	: path(std::move(vpath)),
	shaderType(shaderType),
	sourceType(sourceType)
{
}

ShaderFileInfo::ShaderFileInfo(ShaderFileInfo && other)
	: path(std::move(other.path)),
	shaderType(other.shaderType),
	sourceType(other.sourceType) 
{
	other.shaderType = ShaderFileInfo::none;
	other.sourceType = ShaderFileInfo::unknown;
}

ShaderFileInfo & ShaderFileInfo::operator=(ShaderFileInfo &&other)
{
	path = std::move(other.path);
	shaderType = other.shaderType;
	sourceType = other.sourceType;

	other.shaderType = ShaderFileInfo::none;
	other.sourceType = ShaderFileInfo::unknown;

	return *this;
}

Variable::Variable(const std::string vname,
	const std::string vtypeGLSL,
	size_t vdefinitionOrder,
	VarType vtype,
	int vloc)
	: name(vname),
	typeGLSL(vtypeGLSL),
	definitionOrder(vdefinitionOrder),
	type(vtype),
	location(vloc)
{}

// only name and glsl type are considered for glt_Common.h
bool Variable::operator<(const Variable & other) const
{
	int cmp = name.compare(other.name);
	if (!cmp)
		cmp = typeGLSL.compare(other.typeGLSL);

	return cmp < 0;
}
