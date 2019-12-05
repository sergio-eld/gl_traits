#pragma once

#include <memory>
#include <string>

struct IGeneratorGLT
{
	enum Severity : unsigned char
	{
		find_one,
		find_all,
		find_none
	};

	static std::unique_ptr<IGeneratorGLT> Create(const std::string& sourcePath,
		const std::string& outputPath,
		const std::string& namePredicates, const std::string& extensions,
		Severity = find_one);

	virtual ~IGeneratorGLT() = default;

};
