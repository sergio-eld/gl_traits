#pragma once

#include "IShaderParser.h"

#include <vector>
#include <list>

struct ShaderInfo : public IShaderInfo
{
	std::string filePath;

	std::vector<std::shared_ptr<IVariable>> vao_;
	std::vector<std::shared_ptr<IVariable>> uniforms_;

	ShaderInfo(const std::string& fpath);

	size_t VarsCount(IVariable::VarType type) const override;
};


class ShaderParser : public IShaderParser
{
	struct regexImpl;

	std::string name_;
	std::list<ShaderInfo> shaders_;

	static std::unique_ptr<regexImpl> regImpl_;

	using container_t = decltype(shaders_);


	

public:
	ShaderParser(const std::string& fpath,
		const std::string& namePred,
		const std::string& extensions = std::string());

	const std::string& ProgName() const;
	size_t SourceFilesCount(const std::string& extension) const override;

	std::optional<crIShaderInfo> GetShader(size_t indx,
		const std::string& extension = std::string()) const override;

	void ParseSource(size_t indx,
		const std::string& extension = std::string()) override;


private:

	static container_t GetFilePaths(const std::string& fpath,
		const std::string& namePred, const std::string& extensions);

	static void ParseSource(ShaderInfo& sInf);

};


