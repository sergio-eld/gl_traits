#include "IShaderParser.h"

#include <cassert>

class Variable : public IVariable
{
	std::string name_,
		typeGLSL_;
	//VarType type_;

public:

	Variable(const std::string& name, 
		const std::string& typeGLSL);

	const std::string& Name() const override;
	const std::string& TypeGLSL() const override;

	virtual ~Variable() = default;

};

// exposed to programmer: vertex IN, Uniforms. Can have location.
template <IVariable::VarType type>
class VarPublic : public Variable
{
	int location_;

	static_assert(type == IVariable::uniform ||
		type == IVariable::vertex_in,
		"Invalid VarType");

public:
	VarPublic(const std::string& name,
		const std::string& typeGLSL, int location = -1)
		: Variable(name, typeGLSL),
		location_(location)
	{}

	IVariable::VarType Type() const override
	{
		return type;
	}

	int location() const override
	{
		return location_;
	}

};

// inner variables passed between shaders
template <IVariable::VarType type>
class VarPrivate : public Variable
{
	static_assert(type == IVariable::var_in ||
		type == IVariable::var_out,
		"Invalid VarType");

public:
	VarPrivate(const std::string& name,
		const std::string& typeGLSL)
		: Variable(name, typeGLSL)
	{}

	IVariable::VarType Type() const override
	{
		return type;
	}
};


std::shared_ptr<IVariable> IVariable::Create(const std::string & name,
	const std::string & typeGLSL,
	VarType type, 
	int location)
{
	switch (type)
	{
	case vertex_in:
		return std::make_shared<VarPublic<vertex_in>>(name, typeGLSL, location);
	case uniform:
		return std::make_shared<VarPublic<uniform>>(name, typeGLSL, location);
	case var_in:
		return std::make_shared<VarPrivate<var_in>>(name, typeGLSL);
	case var_out:
		return std::make_shared<VarPrivate<var_out>>(name, typeGLSL);

	default:
		assert(false && "Unhadled case for IVariable::VarType!");
		return std::shared_ptr<IVariable>();
	}
	
}