#pragma once

#include <limits>
#include <string>

struct ISourceFile
{
	enum Type : unsigned char
	{
		text_source,
		text_vertex_shader_source,
		text_fragment_shader_source,
		text_geometry_shader_source,
		// add missing shader types

		header_common,
		header_shader,

		unknown = std::numeric_limits<unsigned char>::max()
	};

	virtual const std::string& Name() const = 0;
	virtual Type FileType() const = 0;

	//virtual void Parse() = 0;

	virtual ~ISourceFile() = default;

	static std::unique_ptr<ISourceFile> Create(std::string&& filePath, Type);
	// iterators

};
