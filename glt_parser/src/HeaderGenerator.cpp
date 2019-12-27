#include "IHeaderGenerator.h"

#include <cassert>
#include <fstream>

bool IHeaderGenerator::compare_less(CRefVariable w1,
    CRefVariable w2)
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

void IHeaderGenerator::AppendSource(const ISourceFile &sf)
{
	sources_.emplace_back(sf);
}

void IHeaderGenerator::CollectVariables(const ISourceFile & sf)
{
	size_t vars = sf.VarsCount();
	for (size_t i = 0; i != vars; ++i)
		vars_.emplace(sf.GetVariable(i));
}

/* Common header must contain:
- all the variables (1 instance for each) from all the shader sources. 
- 
*/
void IHeaderGenerator::GenerateCommonHeader()
{
	PrepareVariablesList();

	fsys::path commonPath{ outputFolder_ },
		commonPathValidate{ outputFolder_ };
	commonPath.append(commonName);
	commonPathValidate.append(commonValidate);

	std::fstream commonHeader{ commonPath, std::fstream::out };

	if (!commonHeader.is_open())
	{
		std::string msg{ "Failed to open file " + commonPath.generic_string() };
		throw std::exception(msg.c_str());
	}

	commonHeader.clear();

	WriteCommonHeaderHead(commonHeader);
	WriteCommonVariables(commonHeader, vars_);

	try
	{
		for (const ISourceFile& sf : sources_)
			WriteShaderTypes(commonHeader, sf);
	}
	catch (const std::exception& e)
	{
		throw e;
	}

	// create validation file

	commonHeader.close();
	commonHeader.open(commonPathValidate, std::fstream::out);
	if (!commonHeader.is_open())
	{
		std::string msg{ "Failed to open file " + commonPathValidate.generic_string() };
		throw std::exception(msg.c_str());
	}

	commonHeader << "#pragma once\n\n#include \"" << commonName << "\"\n\n";
	for (const Variable& var : vars_)
	{
		commonHeader << "static_assert(glt::has_name_v<" << 
			var.name << '_' << var.typeGLSL << ">, \"" << 
			var.name << '_' << var.typeGLSL << " glt_name assertion failed!\");\n"
			"static_assert(glt::has_type_v<" << 
			var.name << '_' << var.typeGLSL << ">, \"" << 
			var.name << '_' << var.typeGLSL << " glt_type assertion failed!\");\n";
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

/*
// input requirements: all sources are unique (unique names)
void IHeaderGenerator::GenerateShaderHeader(const std::vector<CRefISourceFile>& sources) const
{
    // for each source file get vars in and vars out and shader name
    // how to group sources into files??
}*/

void IHeaderGenerator::PrepareVariablesList()
{
	vars_.clear();
	for (CRefISourceFile s : sources_)
		CollectVariables(s);
}

std::basic_ostream<char>& IHeaderGenerator::WriteVariableClassName(std::basic_ostream<char>& os,
	const Variable & var)
{
	os << var.name << '_' << var.typeGLSL;
	return os;
}

void IHeaderGenerator::WriteCommonHeaderHead(std::basic_ostream<char>& file)
{
	assert(file.good());
	file << "#pragma once\n\n#include \"gl_traits.hpp\"\n\n"
		"#define GLSLT_TYPE(NAME, VAR_NAME2, TYPE) struct NAME\\\n"
		"{constexpr static const char* glt_name(){ return #VAR_NAME2;}\\\n"
		"using glt_type = TYPE;};\n\n";
	// TODO: write generation time, user, files and\or license, etc.
}

void IHeaderGenerator::WriteCommonVariables(std::basic_ostream<char>& file,
	const Container& vars)
{
	assert(file.good());
	for (const Variable& var : vars)
	{
		file << "GLSLT_TYPE(";
		WriteVariableClassName(file, var) <<  ", " <<
			var.name << ", " << var.CppGlslType() << ")\n";
	}
}

void IHeaderGenerator::WriteShaderTypes(std::basic_ostream<char>& file, const ISourceFile & sf,
	std::string_view namePredicate)
{
	assert(file.good());
	assert(sf.Name().has_filename());

	size_t totalVars = sf.VarsCount();
	if (!totalVars)
		return;

	std::string filename = sf.Name().stem().generic_string();
	if (namePredicate.empty())
		namePredicate = filename;

	std::vector<CRefVariable> //vertex_In, // vertex_in is same as var_in (?) 
		var_In,
		var_Out,
		var_uniform;

	for (size_t i = 0; i != totalVars; ++i)
	{
		const Variable& var = sf.GetVariable(i);
		switch (var.type)
		{
		case Variable::vertex_in:
			//vertex_In.emplace_back(var);
			//continue;
		case Variable::var_in:
			var_In.emplace_back(var);
			continue;
		case Variable::var_out:
			var_Out.emplace_back(var);
			continue;
		case Variable::uniform:
			var_uniform.emplace_back(var);
			continue;
		default:
			std::string msg = "Variable of unknown type received!" +
				sf.Name().generic_string();
			throw std::exception(msg.c_str());
			break;
		}
	}

	file << "\n\n" << std::string(100, '/') <<
		 "\n//Shader tarits: " << sf.Name().filename().generic_string() << "\n";

	// write even empty sets?
	file << "using VarsIn_" << namePredicate << " = std::tuple<";
	for (const Variable& var : var_In)
		WriteVariableClassName(file, var) << ", ";
	size_t pos = file.tellp();
	file.seekp(pos - 2);
	file << ">;\n";

	// write even empty sets?
	file << "using VarsOut_" << namePredicate << " = std::tuple<";
	for (const Variable& var : var_Out)
		WriteVariableClassName(file, var) << ", ";
	pos = file.tellp();
	file.seekp(pos - 2);
	file << ">;\n";
}
