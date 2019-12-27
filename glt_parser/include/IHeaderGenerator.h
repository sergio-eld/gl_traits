#pragma once

#include "ISourceFile.h"

#include <set>

class IHeaderGenerator
{
	static bool compare_less(CRefVariable w1,
        CRefVariable w2);

	using comp_less = std::integral_constant<decltype(&compare_less),
		&compare_less>;

    constexpr static const char *commonName = "glt_Common.h",
        *commonValidate = "glt_CommonValidate.h",
        *shaderHeaderFolder = "gltShadersGenerated",
        *shaderHeaderPrefix = "gltShader"; // + filename

public:
	using Container = std::set<CRefVariable, comp_less>;


private:
	fsys::path outputFolder_;
	Container vars_;

	std::vector<CRefISourceFile> sources_;

public:
	IHeaderGenerator(fsys::path&& outputFolder);

	void AppendSource(const ISourceFile &sf);

	void CollectVariables(const ISourceFile& sf);
	

	void GenerateCommonHeader();

	size_t VarsCollected() const;

	Container::const_iterator cbegin() const;
	Container::const_iterator cend() const;

    //void GenerateShaderHeader(const std::vector<CRefISourceFile>& sources) const;

private:
	void PrepareVariablesList();

	static std::basic_ostream<char>& WriteVariableClassName(std::basic_ostream<char>&, const Variable& var);
	static void WriteCommonHeaderHead(std::basic_ostream<char>&);
	static void WriteCommonVariables(std::basic_ostream<char>&, const Container&);
	static void WriteShaderTypes(std::basic_ostream<char>&, const ISourceFile&, 
		std::string_view namePredicate = std::string_view());

    static void WriteGeneratedShaderHeader();

};
