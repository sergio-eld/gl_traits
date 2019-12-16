#include "ShaderParser.h"

#include <cassert>
#include <regex>
#include <filesystem>

#include <bitset>

#include <exception>
#include <iostream>

#include <functional>

#include <fstream>
#include <streambuf>
#include <iterator>

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

// TODO: make it a class template!
// helper class to deduce glsl variable type
struct ShaderParser::glsl_variable_info
{
	using Mask = unsigned char;

	// out in loc unif layout
	constexpr static Mask m_empty = 0b00000000,
		m_layout =					0b00000001,
		m_uniform =					0b00000010,
		m_location =				0b00000100,
		m_in =						0b00001000,
		m_out =						0b00010000,

		m_vertex_loc_in_type =		m_layout | m_location | m_in,
		m_vertex_in_type =			m_layout | m_in,
		m_uniform_loc_type =		m_uniform | m_location,
		m_uniform_type =			m_uniform,
		m_var_in_type =				m_in,
		m_var_out_type =			m_out;

	constexpr static size_t layout_slots = 5;

	// std::string name_,
	//	type_;
	// size_t location_;

	Mask mask_;
	IVariable::VarType var_type_;

	glsl_variable_info(// std::string&& name, std::string&& type,
		const bool(&layout_pat)[layout_slots])
		// size_t location = std::numeric_limits<size_t>::max())
		: // name_(std::move(name)),
		// type_(std::move(type)),
		// location_(location),
		mask_(GetMask(layout_pat, std::make_index_sequence<layout_slots>())),
		var_type_(DeduceType(mask_))
	{

	}

	/*
	glsl_variable_info(// const std::string& name, const std::string& type,
		const bool(&layout_pat)[layout_slots])
		// size_t location = std::numeric_limits<size_t>::max())
		: // name_(name),
		// type_(type),
		// location_(location),
		mask_(GetMask(layout_pat, std::make_index_sequence<layout_slots>()))
	{

	}
	*/

	IVariable::VarType GetVarType() const
	{
		return var_type_;
	}

	// TODO: chande this! ITS VERY CLUMSY 
	template <size_t sz, size_t ... indx>
	constexpr static IVariable::VarType DeduceType(const bool(&layout_pat)[sz],
		std::index_sequence<indx...>)
	{
		switch (((layout_pat[indx] << indx) | ...))
		{
		case m_out:
			return IVariable::VarType::var_out;
		case m_in:
			return IVariable::VarType::var_in;
		case m_uniform_type:
		case m_uniform_loc_type:
			return IVariable::VarType::uniform;
		case m_vertex_in_type:
		case m_vertex_loc_in_type:
			return IVariable::VarType::vertex_in;
		default:
			return IVariable::VarType::unknown;
		}
	}


	template <size_t sz, size_t ... indx>
	constexpr static Mask GetMask(const bool(&layout_pat)[sz],
		std::index_sequence<indx...>)
	{
		return ((layout_pat[indx] << indx) | ...);
	}

	static IVariable::VarType DeduceType(Mask mask)
	{
		switch (mask)
		{
		case m_out:
			return IVariable::VarType::var_out;
		case m_in:
			return IVariable::VarType::var_in;
		case m_uniform_type:
		case m_uniform_loc_type:
			return IVariable::VarType::uniform;
		case m_vertex_in_type:
		case m_vertex_loc_in_type:
			return IVariable::VarType::vertex_in;
		default:
			return IVariable::VarType::unknown;
		}
	}

};

struct ShaderParser::ParseImpl
{

	// TODO: modify this regex to be valid!
	inline static std::regex regVarGLSL
	{
		R"((?:(layout)\s+|(uniform)\s+)?)"
		R"((?:[(]location\s*=\s*(\d+)\s*[)]\s+)?)"
		R"((?:(in)\s+|(out)\s+)?)"
		R"((\w+)\s+)"
		R"((\w+)\s*;)"

	};


    ParseImpl()
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

		try 
		{        // assert search cases on creation
			std::vector<std::string> lines{
				{"layout   (location = 0) in  vec3 aPos;"},
			{"layout   in vec2 aTexCoord;"},
			{"uniform mat4 model;"}

			};

			std::vector<IVariable::VarType> var_types
			{
				IVariable::vertex_in,
				IVariable::vertex_in,
				IVariable::uniform
			};

			std::smatch sm;

			for (const std::string& l : lines)
			{
				static size_t itype = 0;
				if (!std::regex_search(l, sm, regVarGLSL))
					throw std::exception(std::string("Critical error : ShaderParser::ParseImpl::"
						" test match case failed!" + l).data());

				bool pat[glsl_variable_info::layout_slots];

				auto iter = ++sm.cbegin();

				for (size_t i = 0; i != glsl_variable_info::layout_slots; ++i)
					pat[i] = (iter++)->matched;

				// std::string type = (iter++)->str(),
				// 	name = (iter++)->str();

				//glsl_variable_info varInfo{ std::move(name),
				//	std::move(type),
				//	pat };

				IVariable::VarType var_type = 
					glsl_variable_info::DeduceType(pat, 
						std::make_index_sequence<glsl_variable_info::layout_slots>());

				if (var_types[itype++] != var_type) //varInfo.GetVarType())
					throw std::exception(std::string("Critical error : ShaderParser::ParseImpl::"
						" invalid var type returned" + l).data());
			}
		}
		catch (const std::exception& e)
		{
			std::cerr << e.what() << std::endl;
			std::terminate();
		}
    }

	// this function assumes that file exists
	void ParseFile(ShaderInfo& shInfo)
	{
		size_t subs = regVarGLSL.mark_count() + 1;

		std::fstream f{ shInfo.filePath, std::fstream::in };
		if (!f.is_open())
			throw std::exception(std::string("Failed to open for parsing: " +
				shInfo.filePath).data());

		std::string fileLoaded{ std::istreambuf_iterator<char>(f),
			std::istreambuf_iterator<char>() };

		// TODO: do not load huge files. Load file only up to "void main()"

		std::sregex_iterator start{ fileLoaded.cbegin(),
			fileLoaded.cend(),
			regVarGLSL },
			end{};

		size_t found = std::distance(start, end);

		while (start != end)
		{
			bool pat[glsl_variable_info::layout_slots];

			const std::smatch& sm = *start;
			auto iter = ++sm.cbegin();

			for (size_t i = 0; i != glsl_variable_info::layout_slots; ++i)
				pat[i] = (iter++)->matched;

			 auto &type = (iter++)->str(),
			 	&name = (iter++)->str();

			IVariable::VarType var_type =
				glsl_variable_info::DeduceType(pat,
					std::make_index_sequence<glsl_variable_info::layout_slots>());

			// TODO: move type deduction to ShaderInfo!
			switch (var_type)
			{
			case IVariable::vertex_in:
				shInfo.vao_.emplace_back(IVariable::Create(name, type, var_type));
				break;
			case IVariable::uniform:
				shInfo.uniforms_.emplace_back(IVariable::Create(name, type, var_type));
				break;
			case IVariable::var_in:
				shInfo.var_in_.emplace_back(IVariable::Create(name, type, var_type));
				break;
			case IVariable::var_out:
				shInfo.var_out.emplace_back(IVariable::Create(name, type, var_type));
				break;
			case IVariable::unknown:
				throw std::exception("ShaderParser::ParseFile Invalid Variable type receved");
			default:
				break;
			}
			++start;
		}

	}

};

std::unique_ptr<ShaderParser::ParseImpl> ShaderParser::parseImpl_ = std::make_unique<ShaderParser::ParseImpl>();


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
	parseImpl_->ParseFile(sInf);
    //////////////////////////////////////
}


std::unique_ptr<IShaderParser> IShaderParser::Create(const std::string & fpath,
	const std::string & namePred, 
	const std::string & extensions)
{
	return std::make_unique<ShaderParser>(fpath, namePred, extensions);
}


