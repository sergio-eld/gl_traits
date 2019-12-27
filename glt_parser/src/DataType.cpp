#include "IDataType.h"

#include <map>
#include <bitset>
#include <regex>

#include <cassert>

std::string_view ShaderFileInfo::cpp_glt_shader_type(ShaderType st)
{
    using namespace std::literals;
    static std::map<ShaderType, std::string_view> types{
        {shader_compute, "glt::ShaderTarget::compute"sv},
        {shader_vertex, "glt::ShaderTarget::vertex"sv},
        {shader_tess_control, "glt::ShaderTarget::tess_control"sv},
        {shader_tess_evaluation, "glt::ShaderTarget::tess_evaluation"sv},
        {shader_geometry, "glt::ShaderTarget::geometry"sv},
        {shader_fragment, "glt::ShaderTarget::fragment"sv}
    };

    auto found = types.find(st);
    assert(found != types.cend());
    
    return found->second;
}

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

Variable::Variable(const std::string& vname,
	const std::string& vtypeGLSL,
	size_t vdefinitionOrder,
	VarType vtype,
	int vloc)
	: name(vname),
    typeGLSL(vtypeGLSL),
    glslDataType(get_glsl_type(typeGLSL)),
	definitionOrder(vdefinitionOrder),
	type(vtype),
	location(vloc)
{}

// only name and glsl type are considered for glt_Common.h
bool Variable::operator<(const Variable & other) const
{
    return std::tie(name, typeGLSL) < std::tie(other.name, other.typeGLSL);
}

bool Variable::Valid() const
{
    return !name.empty() && !typeGLSL.empty();
}

/* glsl types:
0. Scalars: bool, int, uint, float, double
1. vectors: bvecn, ivecn, uvecn, vecn, dvecn
2. matrices: matnxm, matn, dmat
3. samples and images
4. user-defined structs
*/

std::string Variable::CppGlslType() const
{
	return cpp_glsl_type(glslDataType, typeGLSL);
}

Variable::GLSLDataType Variable::get_glsl_type(const std::string & rawtypeGLSL)
{
    static std::regex patt{ R"((bool|int|uint|float|double)|)" // scalar
        R"((bvec\d|ivec\d|uvec\d|vec\d|dvec\n)|)" // vector
        R"((mat\dx\d|mat\d|dmat\d)|)" // matrix
        R"(([iu]?sampler(?:[1-3]D|Buffer|Cube|CubeArray|2DRect|[12]DArray|2DMS|2DMSArray))|)" // sampler
        R"(([iu]?image(?:[1-3]D|Buffer|Cube|CubeArray|2DRect|[12]DArray|2DMS|2DMSArray)))" }; // image

    static std::smatch sm;

    if (!std::regex_match(rawtypeGLSL, sm, patt))
        return user_defined;

    size_t group = 0;

    std::smatch::const_iterator subIter = sm.cbegin();

    unsigned char matchedGroup = user_defined;
    while (++subIter != sm.cend() && !matchedGroup)
    {
        if (!subIter->matched)
        {
            ++group;
            continue;
        }
        matchedGroup |= 1 << group;
    }

    assert(std::bitset<8>(matchedGroup).count() == 1 && "Invalid flag received!");
    return GLSLDataType(matchedGroup);
}

std::string Variable::cpp_glsl_type(GLSLDataType dataType, const std::string& rawtypeGLSL)
{
    switch (dataType)
    {
    case scalar:
    case vector:
    case matrix:
        return "glm::" + rawtypeGLSL;
    case sampler:
    case image:
        return "int";
    default:
        throw std::exception("Failed to deduce glsl data type!");
    }
}
