#include "ShaderParser.h"

#include <cassert>
#include <regex>
#include <filesystem>

#include <exception>
#include <iostream>

namespace fsys = std::filesystem;

ShaderInfo::ShaderInfo(const std::string & fpath)
	: filePath(fpath)
{}

size_t ShaderInfo::VarsCount(IVariable::VarType type) const
{
	switch (type)
	{
	case IVariable::vertex_in:
		return vao_.size();
	case IVariable::uniform:
		return uniforms_.size();
	default:
		return 0;
	}
}

struct ShaderParser::regexImpl
{
	inline static std::string layout_uniform{
		R"(((layout)|(uniform)\s+(?:[(]location\s{1,5}=\s{1,5}(\d{1,3})[)])?))" };

	inline static std::regex glslVariable
	{
		layout_uniform
	};

    regexImpl()
    {
        static size_t entities = 0;

        if (entities++)
        {
            std::set_terminate([]() 
            {
                std::cerr << "Critical error: ShaderParser::regexImpl::Only one instance is allowed" << std::endl;
                std::abort();
            });

            std::terminate();
        }

        // assert search cases on creation
        std::vector<std::string> lines{
            {"layout (location = 0) in vec3 aPos;"},
        {"layout   in vec2 aTexCoord;"},
        {"uniform mat4 model;"}

        };

        std::smatch sm;

        for (const std::string& l : lines)
        {
            bool matched = std::regex_search(l, sm, glslVariable);
        }
    }

};

std::unique_ptr<ShaderParser::regexImpl> ShaderParser::regImpl_ = std::make_unique<ShaderParser::regexImpl>();


ShaderParser::ShaderParser(const std::string & fpath, const std::string & namePred, const std::string & extensions)
	: name_(namePred),
	shaders_(GetFilePaths(fpath, namePred, extensions))
{}

const std::string & ShaderParser::ProgName() const
{
	return name_;
}

size_t ShaderParser::SourceFilesCount(const std::string & extension) const
{	
	if (extension.empty())
		return shaders_.size();

	/*
	// TODO: implement case if extensions not empty
	return sources_.size();
	*/
	return 0;
}

std::optional<crIShaderInfo> ShaderParser::GetShader(size_t indx, const std::string & extension) const
{
	// TODO: encapsulate implementation!
	if (!extension.empty())
		assert(false && "Extensions not implemented yet");

	if (indx >= shaders_.size())
		return std::nullopt;

	auto iter = std::next(shaders_.cbegin(), indx);

	return crIShaderInfo(*iter);
}

void ShaderParser::ParseSource(size_t indx, const std::string & extension)
{
	// TODO: encapsulate implementation!
	if (!extension.empty())
		assert(false && "Extensions not implemented yet");

	if (indx >= shaders_.size())
	{
		assert(false && "Index is out of range!");
		throw std::exception("ShaderParser::ParseSourse::Index is out of range!");
	}

	auto iter = std::next(shaders_.begin(), indx);

	return ParseSource(*iter);

}

ShaderParser::container_t ShaderParser::GetFilePaths(const std::string & fpath, const std::string & namePred, const std::string & extensions)
{
	fsys::path sourceDir{ fpath };
	if (!fsys::is_directory(sourceDir))
		sourceDir = sourceDir.parent_path();

	// these all must be checked outside the constructor!
	assert(fsys::exists(sourceDir) && "Source directory doesn't exist!");
	assert(!fsys::is_empty(sourceDir) && "Source directory is empty!");
	assert(!namePred.empty() && "Name Predicate is empty!");
	assert(namePred.find(' ') == std::string::npos && 
		"Only one name predicate is allowed!");

	std::string ext{ R"((?:.*))" };
	if (!extensions.empty())
	{
		ext.clear();

		std::regex regExt{ R"(.\w+)" };
		auto ext_begin = std::sregex_iterator(extensions.cbegin(), extensions.cend(), regExt);
		auto ext_end = std::sregex_iterator();

		for (std::sregex_iterator i = ext_begin; i != ext_end; ++i)
			ext += "|(?:" + i->str() + ")";

	}
	std::string patName{ '(' + namePred + R"(\w*)()" + ext + ')' };
	std::regex regName{ patName, std::regex::icase };
	std::smatch sm;

	container_t out;

	for (auto& p : fsys::directory_iterator(sourceDir))
	{
		if (!p.is_regular_file())
			continue;

		std::string path = p.path().generic_string();
		if (std::regex_search(path, sm, regName))
			out.push_back(ShaderInfo(p.path().generic_string()));
	}

	return out;
}

void ShaderParser::ParseSource(ShaderInfo & sInf)
{
    //////////////////////////////////////

    //////////////////////////////////////
}


std::unique_ptr<IShaderParser> IShaderParser::Create(const std::string & fpath,
	const std::string & namePred, 
	const std::string & extensions)
{
	return std::make_unique<ShaderParser>(fpath, namePred, extensions);
}


