#pragma once

#include <limits>
#include <string>
#include <string_view>
#include <vector>

#include <filesystem>

namespace fsys = std::filesystem;

struct ShaderFileInfo
{
	enum ShaderType : unsigned char
	{
		shader_vertex,
		shader_fragment,
		shader_geometry,

		shader_compute,

		none = std::numeric_limits<unsigned char>::max()
	};

	enum SourceType : unsigned char
	{
		text_source,
		header_common,

		unknown = std::numeric_limits<unsigned char>::max()
	};

	static inline std::vector<ShaderType> list_types{ shader_vertex,
		shader_fragment,
		shader_geometry,
		shader_compute};

	struct ShaderExtensionInfo
	{
		std::string extension;
		ShaderType type;

		bool operator<(const ShaderExtensionInfo& other) const
		{
			return extension < other.extension
				|| type < other.type;
		}
	};

	ShaderFileInfo(fsys::path&& path, const ShaderExtensionInfo& type);
	ShaderFileInfo(fsys::path&& path, ShaderType shaderType, SourceType sourceType);

	ShaderFileInfo(const ShaderFileInfo& other) = default;
	ShaderFileInfo(ShaderFileInfo&& other);

	ShaderFileInfo& operator=(const ShaderFileInfo&) = default;
	ShaderFileInfo& operator=(ShaderFileInfo&&);

	fsys::path path;
	ShaderType shaderType;
	SourceType sourceType;
};

struct Variable
{
	enum VarType : unsigned char
	{
		vertex_in,
		uniform,
		var_in,
		var_out,

		unknown
	};

    // TODO: add arrays
    enum GLSLDataType : unsigned char
    {
        user_defined = 0,
        scalar = 1,
        vector = 2,
        matrix = 4,
        sampler = 6,
        image = 8
    };

    std::string name,
        typeGLSL;
    GLSLDataType glslDataType;
	size_t definitionOrder;
	VarType type;
	int location;


	Variable(const std::string& vname,
		const std::string& vtypeGLSL,
		size_t vdefinitionOrder,
		VarType vtype,
		int vloc = -1);

	Variable(const Variable&) = default;
	Variable(Variable&&) = default;

	bool operator<(const Variable& other) const;
    bool Valid() const;
    operator bool() const
    {
        return Valid();
    }

    static GLSLDataType get_glsl_type(const std::string& rawtypeGLSL);
    static std::string cpp_glsl_type(GLSLDataType, const std::string& rawtypeGLSL);

};