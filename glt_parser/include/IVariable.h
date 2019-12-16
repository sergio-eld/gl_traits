#pragma once

#include <string>

struct IVariable
{
	enum VarType : unsigned char
	{
		vertex_in,
		uniform,
		var_in,
		var_out,

		unknown
	};

	virtual const std::string& Name() const = 0;
	virtual const std::string& TypeGLSL() const = 0;
	virtual VarType Type() const = 0;

	// only for vertex_in and uniforms (?)
	virtual int location() const
	{
		return -1;
	}

	static std::unique_ptr<IVariable> Create(const std::string& name,
		const std::string& typeGLSL,
		VarType type,
		int location = -1);

	virtual ~IVariable() = default;

};
