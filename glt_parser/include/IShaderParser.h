#pragma once

#include <string>
#include <optional>
#include <string_view>

#include "IVariable.h"

struct IShaderInfo
{
	enum ShaderType : unsigned char
	{
		unknown
	};

	virtual size_t VarsCount(IVariable::VarType) const = 0;

	// TODO: add shader type deduction
	virtual ShaderType GetType() const
	{
		return unknown;
	}

};

using crIShaderInfo = std::reference_wrapper<const IShaderInfo>;

struct IShaderParser
{
	virtual const std::string& ProgName() const = 0;

	virtual size_t SourceFilesCount(const std::string& extension =
		std::string()) const = 0;

	virtual std::optional<crIShaderInfo> GetShader(size_t indx, 
		const std::string& extension = std::string()) const = 0;

	virtual void ParseSource(size_t indx,
		const std::string& extension = std::string()) = 0;

	virtual void ParseAll()
	{
		size_t count = SourceFilesCount();
		for (size_t i = 0; i != count; ++i)
			ParseSource(i);
	}

	static std::unique_ptr<IShaderParser> Create(const std::string& fpath,
		const std::string& namePred,
		const std::string& extensions = std::string());

	virtual ~IShaderParser() = default;

};

