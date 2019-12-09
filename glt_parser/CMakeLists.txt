cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

project(glt_parser LANGUAGES CXX)


add_executable(${PROJECT_NAME}
	src/${PROJECT_NAME}.cpp
	src/Arguments.cpp
	src/ComLineParser.cpp
	
	src/ShaderParser.cpp
	src/ShaderParser.h

	src/Variable.h
	src/Variable.cpp
	
	src/GeneratorGLT.cpp
	
	include/IParser.h
	include/IShaderParser.h
	include/IGeneratorGLT.h

	)
	
target_include_directories(${PROJECT_NAME}
	PRIVATE
		${PROJECT_SOURCE_DIR}/include
	)
	
set(ARGUMENTS
	"-s C:/Users/KolesnikSV/Documents/DEV2/gl_traits/glt_parser/sandbox \
-d C:/Users/KolesnikSV/Documents/dev_build/gl_traits/bin/Debug \
-p \"Shader Compute\" \
-e \".vs .fs .cs .vshader .fshader .cshader\" \
-w warnings"
	)
	
set_target_properties(${PROJECT_NAME}
	PROPERTIES
		CXX_STANDARD 17
		CXX_STANDARD_REQUIRED True
		VS_DEBUGGER_COMMAND_ARGUMENTS ${ARGUMENTS}
	)
	
add_subdirectory(marconi)