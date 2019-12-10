#include "Variable.h"

Variable::Variable(const std::string & name, const std::string & typeGLSL)
	: name_(name),
	typeGLSL_(typeGLSL)
{}

const std::string & Variable::Name() const
{
	return name_;
}

const std::string & Variable::TypeGLSL() const
{
	return typeGLSL_;
}
