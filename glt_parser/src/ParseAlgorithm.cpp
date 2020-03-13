#include "ParseAlgorithm/ParseAlgorithm.h"

#include <filesystem>

#include <fstream>
#include <streambuf>
#include <regex>

#include <cassert>

namespace fsys = std::filesystem;

struct glsl_variable_info
{
	using Mask = unsigned char;

	// out in loc unif layout
	constexpr static Mask m_empty = 0b00000000,
		m_layout = 0b00000001,
		m_uniform = 0b00000010,
		m_location = 0b00000100,
		m_in = 0b00001000,
		m_out = 0b00010000,

		m_vertex_loc_in_type = m_layout | m_location | m_in,
		m_vertex_in_type = m_layout | m_in,
		m_uniform_loc_type = m_uniform | m_location,
		m_uniform_type = m_uniform,
		m_var_in_type = m_in,
		m_var_out_type = m_out;

	constexpr static size_t layout_slots = 5;

	Mask mask_;
	Variable::VarType var_type_;

	glsl_variable_info(const bool(&layout_pat)[layout_slots])
		: mask_(GetMask(layout_pat, std::make_index_sequence<layout_slots>())),
		var_type_(DeduceType(mask_))
	{

	}

	Variable::VarType GetVarType() const
	{
		return var_type_;
	}

	// TODO: chande this! ITS VERY CLUMSY 
	template <size_t sz, size_t ... indx>
	constexpr static Variable::VarType DeduceType(const bool(&layout_pat)[sz],
		std::index_sequence<indx...>)
	{
		// form the mask from bools
		switch (((layout_pat[indx] << indx) | ...))
		{
		case m_out:
			return Variable::VarType::var_out;
		case m_in:
			return Variable::VarType::var_in;
		case m_uniform_type:
		case m_uniform_loc_type:
			return Variable::VarType::uniform;
		case m_vertex_in_type:
		case m_vertex_loc_in_type:
			return Variable::VarType::vertex_in;
		default:
			return Variable::VarType::unknown;
		}
	}


	template <size_t sz, size_t ... indx>
	constexpr static Mask GetMask(const bool(&layout_pat)[sz],
		std::index_sequence<indx...>)
	{
		return ((layout_pat[indx] << indx) | ...);
	}

	static Variable::VarType DeduceType(Mask mask)
	{
		switch (mask)
		{
		case m_out:
			return Variable::VarType::var_out;
		case m_in:
			return Variable::VarType::var_in;
		case m_uniform_type:
		case m_uniform_loc_type:
			return Variable::VarType::uniform;
		case m_vertex_in_type:
		case m_vertex_loc_in_type:
			return Variable::VarType::vertex_in;
		default:
			return Variable::VarType::unknown;
		}
	}

};


template <>
std::vector<Variable>
ParseAlgorithm<ShaderFileInfo::text_source>::ParseImpl(const fsys::path& filePath)
{

	std::fstream f{ filePath, std::fstream::in };
	if (!f.is_open())
		throw std::exception(std::string("Failed to open for parsing: " +
			filePath.generic_string()).data());

	std::string fileLoaded{ std::istreambuf_iterator<char>(f),
			std::istreambuf_iterator<char>() };

	return ParseImpl(std::string_view(fileLoaded));
}


template <>
std::vector<Variable>
ParseAlgorithm<ShaderFileInfo::header_common>::ParseImpl(const fsys::path& filePath)
{
	return std::vector<Variable>();
}

template<>
std::vector<Variable> ParseAlgorithm<ShaderFileInfo::text_source>::ParseImpl(std::string_view shaderSource)
{
	std::vector<Variable> out;
	static std::regex regVarGLSL
	{
		R"((?:(layout)\s+|(uniform)\s+)?)"
		R"((?:[(]location\s*=\s*(\d+)\s*[)]\s+)?)"
		R"((?:(in)\s+|(out)\s+)?)"
		R"((\w+)\s+)"
		R"((\w+)\s*;)"
	};

	static size_t subs = regVarGLSL.mark_count() + 1;

	std::regex_iterator<std::string_view::iterator> start{ shaderSource.cbegin(),
		shaderSource.cend(),
		regVarGLSL },
		end{};

	size_t found = std::distance(start, end);

	size_t definitionOrder = 0;

	while (start != end)
	{
		bool pat[glsl_variable_info::layout_slots];

		const std::match_results<std::string_view::const_iterator>& sm = *start;
		auto iter = ++sm.cbegin();

		for (size_t i = 0; i != glsl_variable_info::layout_slots; ++i)
			pat[i] = (iter++)->matched;

		auto &type = (iter++)->str(),
			&name = (iter++)->str();

		int location = -1;

		try
		{
			if (pat[2])
				location = std::stoi(sm[3].str());
		}
		catch (...)
		{
			throw std::exception("Invalid value for location received!");
		}

		Variable::VarType var_type =
			glsl_variable_info::DeduceType(pat,
				std::make_index_sequence<glsl_variable_info::layout_slots>());
		if (var_type == Variable::unknown)
		{
			assert(false && "Unknown variable type received!");
			throw std::exception("Unknown variable type received!");
		}

		out.emplace_back(name, type, definitionOrder++, var_type, location);
		++start;
	}

	return out;
}


template<>
std::vector<Variable> ParseAlgorithm<ShaderFileInfo::header_common>::ParseImpl(std::string_view shaderSource)
{
	return std::vector<Variable>();
}


std::unique_ptr<IParseAlgorithm> IParseAlgorithm::GetAlgorithm(ShaderFileInfo::SourceType sourceType)
{
	switch (sourceType)
	{
	case ShaderFileInfo::text_source:
		return std::make_unique<ParseAlgorithm<ShaderFileInfo::text_source>>();
	case ShaderFileInfo::header_common:
		return std::make_unique<ParseAlgorithm<ShaderFileInfo::header_common>>();
	default:
		assert(false && "Unknown shader source file!");
		return nullptr;
	}
}

/*
std::unique_ptr<IParseAlgorithm> IParseAlgorithm::GetAlgorithm(ShaderFileInfo::ShaderType)
{
	return std::make_unique<ParseAlgorithm<ShaderFileInfo::text_source>>();
}*/

template class ParseAlgorithm<ShaderFileInfo::text_source>;
template class ParseAlgorithm<ShaderFileInfo::header_common>;