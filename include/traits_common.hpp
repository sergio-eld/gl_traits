#pragma once

//common gl traits

typedef cexpr_generic_map<
	cexpr_pair<GLbyte, auto_t<GL_BYTE>>,
	cexpr_pair<GLubyte, auto_t<GL_UNSIGNED_BYTE>>,
	cexpr_pair<GLshort, auto_t<GL_SHORT>>,
	cexpr_pair<GLushort, auto_t<GL_UNSIGNED_SHORT>>,
	cexpr_pair<GLint, auto_t<GL_INT>>,
	cexpr_pair<GLuint, auto_t<GL_UNSIGNED_INT>>,
	cexpr_pair<GLfixed, auto_t<GL_FIXED>>,
	cexpr_pair<GLhalf, auto_t<GL_HALF_FLOAT>>,
	cexpr_pair<GLfloat, auto_t<GL_FLOAT>>,
	cexpr_pair<GLdouble, auto_t<GL_DOUBLE>>
>
gl_types_map;

template <int gl_val>
using gl_value = std::integral_constant<int, gl_val>;

//These wrappers serve to enforse usage of predefined parameters in functions that accept only curtain values
struct gl_none : public gl_value<GL_NONE> {};
struct gl_lequal : public gl_value<GL_LEQUAL> {};
struct gl_gequal : public gl_value<GL_GEQUAL> {};
struct gl_less : public gl_value<GL_LESS> {};
struct gl_greater : public gl_value<GL_GREATER> {};
struct gl_equal : public gl_value<GL_EQUAL> {};
struct gl_notequal : public gl_value<GL_NOTEQUAL> {};
struct gl_always : public gl_value<GL_ALWAYS> {};
struct gl_never : public gl_value<GL_NEVER> {};

