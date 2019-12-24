cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

project(glt_parser LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRES True)

add_library(IDataType
	OBJECT
		include/IDataType.h
		src/DataType.cpp
	)

target_include_directories(IDataType 
	PRIVATE
		${PROJECT_SOURCE_DIR}/include
	)	


add_library(FolderScanner
	OBJECT
		include/FolderScanner.h
		src/FolderScanner.cpp
	)
	
target_include_directories(FolderScanner 
	PRIVATE
		${PROJECT_SOURCE_DIR}/include
	)

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

add_library(HeaderGenerator
	OBJECT
		include/IHeaderGenerator.h
		src/HeaderGenerator.cpp
	)

target_include_directories(HeaderGenerator 
	PRIVATE
		${PROJECT_SOURCE_DIR}/include
	)

# test ParseAlgorithm
add_executable(test_ParseAlgorithm
		sandbox/test_ParseAlgorithm.cpp
	
		$<TARGET_OBJECTS:ParseAlgorithm>
	)
	
target_link_libraries(test_ParseAlgorithm
	PRIVATE
		IDataType
	)
	

target_include_directories(test_ParseAlgorithm 
	PRIVATE
		${PROJECT_SOURCE_DIR}/include
		${PROJECT_SOURCE_DIR}/src
	)

set_target_properties(test_ParseAlgorithm
	PROPERTIES
		VS_DEBUGGER_COMMAND_ARGUMENTS ${PROJECT_SOURCE_DIR}/sandbox/shader_vertex.vs
	)

# test FolderScanner
add_executable(test_FolderScanner
		sandbox/test_FolderScanner.cpp
	
		$<TARGET_OBJECTS:FolderScanner>
	)
	
target_include_directories(test_FolderScanner 
	PRIVATE
		${PROJECT_SOURCE_DIR}/include
		${PROJECT_SOURCE_DIR}/src
	)

target_link_libraries(test_FolderScanner
	PRIVATE
		IDataType
	)	


set_target_properties(test_FolderScanner
	PROPERTIES
		VS_DEBUGGER_COMMAND_ARGUMENTS "${PROJECT_SOURCE_DIR}/sandbox/ \".vs .fs .gs\""
	) 


# main app
add_executable(${PROJECT_NAME}
	src/${PROJECT_NAME}.cpp
	src/Arguments.cpp
	src/ComLineParser.cpp

	$<TARGET_OBJECTS:IDataType>
	$<TARGET_OBJECTS:FolderScanner>
	
	$<TARGET_OBJECTS:SourceFile>
	$<TARGET_OBJECTS:ParseAlgorithm>
	$<TARGET_OBJECTS:HeaderGenerator>
	)
	
target_include_directories(${PROJECT_NAME}
	PRIVATE
		${PROJECT_SOURCE_DIR}/include
	)
	
set(ARGUMENTS
	"-s ${PROJECT_SOURCE_DIR}/sandbox/ \
-d ${PROJECT_BINARY_DIR}/Debug/ \
-p \"Shader Compute\" \
-e \".vs .fs .gs\" \
-w warnings"
	)
	
set_target_properties(${PROJECT_NAME}
	PROPERTIES
		CXX_STANDARD 17
		CXX_STANDARD_REQUIRED True
		VS_DEBUGGER_COMMAND_ARGUMENTS ${ARGUMENTS}
		
		#RUNTIME_OUTPUT_DIRECTORY bin
	
	)