cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

# TODO: must have an install command and export Release version for glt_parser, Release and Debug Version for ParseAlgorithmPublic (for shader text sources for now) static library

project(glt_parser LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRES True)

add_library(CLParser
	OBJECT
		include/IParser.h

		src/Arguments.cpp
		src/ComLineParser.cpp
		
		src/ArgShaderExtensions.h
		src/ArgShaderExtensions.cpp
	)
	
target_include_directories(CLParser
	PRIVATE
		${PROJECT_SOURCE_DIR}/src
		${PROJECT_SOURCE_DIR}/include
	)

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
		include/ParseAlgorithm/IParseAlgorithm.h
		include/ParseAlgorithm/ParseAlgorithm.h
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

	$<TARGET_OBJECTS:CLParser>

	$<TARGET_OBJECTS:IDataType>
	$<TARGET_OBJECTS:FolderScanner>
	
	$<TARGET_OBJECTS:SourceFile>
	$<TARGET_OBJECTS:ParseAlgorithm>
	$<TARGET_OBJECTS:HeaderGenerator>
	)
	
#target_link_libraries(${PROJECT_NAME}
#	PRIVATE
#		CLParser
#	)
	
target_include_directories(${PROJECT_NAME}
	PRIVATE
		${PROJECT_SOURCE_DIR}/include
	)
	
set(ARGUMENTS
	"-s ${PROJECT_SOURCE_DIR}/sandbox/ \
-d ${PROJECT_BINARY_DIR}/Debug/ \
--vert \".vshader .vs\" \
--frag \".fshader .fs\" \
--geom \".gshader .gs\" \
--comp \".cshader .cs\""
	) 
	
set_target_properties(${PROJECT_NAME}
	PROPERTIES
		CXX_STANDARD 17
		CXX_STANDARD_REQUIRED True
		VS_DEBUGGER_COMMAND_ARGUMENTS ${ARGUMENTS}
		
		DEBUG_POSTFIX "_d"
		OUTPUT_NAME_RelWithDebInfo ${PROJECT_NAME}_r_d
		OUTPUT_NAME_MinSizeRel ${PROJECT_NAME}_r_min
		#RUNTIME_OUTPUT_DIRECTORY bin
	)
	
# module for export
add_library(ParseAlgorithmPublic
	STATIC
		$<TARGET_OBJECTS:ParseAlgorithm>
		$<TARGET_OBJECTS:IDataType>
	)

target_include_directories(ParseAlgorithmPublic
	PUBLIC
		$<INSTALL_INTERFACE:include> # copy headers IDataType.h, IParseAlgorithm.h and ParseAlgorithm.h
	PRIVATE
		${PROJECT_SOURCE_DIR}/include
	)
	
set_target_properties(ParseAlgorithmPublic
	PROPERTIES
		CXX_STANDARD 17
		CXX_STANDARD_REQUIRED True
		DEBUG_POSTFIX "_d"
		OUTPUT_NAME_RelWithDebInfo ParseAlgorithmPublic_r_d
		OUTPUT_NAME_MinSizeRel ParseAlgorithmPublic_r_min
		
		ARCHIVE_OUTPUT_DIRECTORY lib
		LIBRARY_OUTPUT_DIRECTORY lib
	)
	
# installation

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(CMAKE_INSTALL_PREFIX "${PROJECT_BINARY_DIR}/install" CACHE PATH "Default Installation folder" FORCE)
endif()


set(config_install_dir ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})
set(version_config ${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake)
set(project_config ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake)
set(targets_export_name ${PROJECT_NAME}Targets)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)
message(STATUS "${PROJECT_NAME} will be installed to ${CMAKE_INSTALL_PREFIX}")

configure_package_config_file(
		${PROJECT_SOURCE_DIR}/config/${PROJECT_NAME}Config.cmake.in
		${project_config}
	INSTALL_DESTINATION
		${config_install_dir}
	)
	
install(TARGETS
		${PROJECT_NAME}
		ParseAlgorithmPublic
	CONFIGURATIONS
		#Debug
		#RelWithDebInfo
		Release
		#MinSizeRel
	EXPORT 
		${targets_export_name}
	RUNTIME
		DESTINATION ${CMAKE_INSTALL_BINDIR}
	LIBRARY
		DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE 
		DESTINATION ${CMAKE_INSTALL_LIBDIR}
	)
	
message(STATUS "Installing headers to ${CMAKE_INSTALL_INCLUDEDIR}")
install(FILES 
		${PROJECT_SOURCE_DIR}/include/IDataType.h
		${PROJECT_SOURCE_DIR}/include/ParseAlgorithm/ParseAlgorithm.h
		${PROJECT_SOURCE_DIR}/include/ParseAlgorithm/IParseAlgorithm.h
	DESTINATION
		${CMAKE_INSTALL_INCLUDEDIR}
	)

install(EXPORT
		${targets_export_name}
	DESTINATION
		${config_install_dir}
	)

install(FILES
		${project_config}
		#${version_config}
	DESTINATION
		${config_install_dir}
	)
