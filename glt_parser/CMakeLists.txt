cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

project(glt_parser LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRES True)

add_library(ParseAlgorithm
	OBJECT
		include/IParseAlgorithm.h
		src/ParseAlgorithm.h
		src/ParseAlgorithm.cpp
	)

target_include_directories(ParseAlgorithm 
	PRIVATE
		${PROJECT_SOURCE_DIR}/include
	)
		
add_library(SourceFile
	OBJECT
		include/ISourceFile.h
		src/SourceFile.h
		src/SourceFile.cpp
	)

target_include_directories(SourceFile 
	PRIVATE
		${PROJECT_SOURCE_DIR}/include
	)
	
# consider removing layers of inheritence, leave one class for data holding?
add_library(Variable
	OBJECT
		include/IVariable.h
		src/Variable.h
		src/Variable.cpp
	)

target_include_directories(Variable 
	PRIVATE
		${PROJECT_SOURCE_DIR}/include
	)

# test ParseAlgorithm
add_executable(test_ParseAlgorithm
		sandbox/test_ParseAlgorithm.cpp
	
		$<TARGET_OBJECTS:ParseAlgorithm>
		$<TARGET_OBJECTS:Variable>
	)
	
target_include_directories(test_ParseAlgorithm 
	PRIVATE
		${PROJECT_SOURCE_DIR}/include
		${PROJECT_SOURCE_DIR}/src
	)

# main app
add_executable(${PROJECT_NAME}
	src/${PROJECT_NAME}.cpp
	src/Arguments.cpp
	src/ComLineParser.cpp
	
	src/ShaderParser.cpp
	src/ShaderParser.h

	
	$<TARGET_OBJECTS:Variable>
	$<TARGET_OBJECTS:SourceFile>
	$<TARGET_OBJECTS:ParseAlgorithm>
	
	
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