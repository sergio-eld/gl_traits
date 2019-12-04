cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

project(glt_parser LANGUAGES CXX)


add_executable(${PROJECT_NAME}
	src/${PROJECT_NAME}.cpp
	src/Arguments.cpp
	src/ComLineParser.cpp
	
	include/IParser.h
	)
	
target_include_directories(${PROJECT_NAME}
	PRIVATE
		${PROJECT_SOURCE_DIR}/include
	)
	
set(ARGUMENTS
	"-s C:/Users/KolesnikSV/Documents/DEV2/gl_traits/glt_parser/test_shader \
-d C:/Users/KolesnikSV/Documents/dev_build/gl_traits/bin/Debug \
-p \"Test Compute\" \
-e \".vs .fs .cs .vshader .fshader .cshader\" \
-w warnings"
	)
	
set_target_properties(${PROJECT_NAME}
	PROPERTIES
		CXX_STANDARD 17
		CXX_STANDARD_REQUIRED True
		VS_DEBUGGER_COMMAND_ARGUMENTS ${ARGUMENTS}
	)
	
