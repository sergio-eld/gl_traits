cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(gl_traits LANGUAGES CXX)

# TODO: for BUILD_INTERFACE install glt_parser to inner folder. For INSTALL_INTERFACE install release glt_parser 

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}/glt_parser)
execute_process(COMMAND ${CMAKE_COMMAND} -G ${CMAKE_GENERATOR}
					-S ${PROJECT_SOURCE_DIR}/glt_parser -B ${PROJECT_BINARY_DIR}/glt_parser/build
					-A x64 
					-DCMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE=${PROJECT_BINARY_DIR}/glt_parser
					-DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE=${PROJECT_BINARY_DIR}/glt_parser
					-DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG=${PROJECT_BINARY_DIR}/glt_parser
					-DCMAKE_DEBUG_POSTFIX=_d
					-DCMAKE_BUILD_TYPE=Release
				)
execute_process(COMMAND ${CMAKE_COMMAND} --build ${PROJECT_BINARY_DIR}/glt_parser/build --target glt_parser --config Release -- /nologo /verbosity:minimal)
#execute_process(COMMAND ${CMAKE_COMMAND} --build ${PROJECT_BINARY_DIR}/glt_parser/build --target ParseAlgorithmPublic --config Release -- /nologo /verbosity:minimal)
execute_process(COMMAND ${CMAKE_COMMAND} --build ${PROJECT_BINARY_DIR}/glt_parser/build --target ParseAlgorithmPublic --config Debug -- /nologo /verbosity:minimal)

find_library(SHADER_SOURCE_PARSER ParseAlgorithmPublic_d.lib ${PROJECT_BINARY_DIR}/glt_parser/ REQUIRED) # this shit is buggy and does not work despite EXISTS returning True!!!!
find_file(SHADER_SOURCE_PARSER_HEADER ParseAlgorithm.h ${PROJECT_SOURCE_DIR}/glt_parser/include REQUIRED)
find_program(GLT_GENERATOR ${PROJECT_BINARY_DIR}/glt_parser/glt_parser.exe REQUIRED)

message(STATUS "${SHADER_SOURCE_PARSER}")
set(SHADER_SOURCE_PARSER ${PROJECT_BINARY_DIR}/glt_parser/ParseAlgorithmPublic_d.lib)
if (EXISTS ${SHADER_SOURCE_PARSER})
message(STATUS "Found ${SHADER_SOURCE_PARSER}")
else()
message(SEND_ERROR "Failed to find ParseAlgorithmPublic_d.lib")
endif()

message(STATUS "${SHADER_SOURCE_PARSER_HEADER}")
if (SHADER_SOURCE_PARSER_HEADER)
message(STATUS "Found ${SHADER_SOURCE_PARSER_HEADER}")
else()
message(SEND_ERROR "Failed to find IParseAlgorithm.h")
endif()

get_filename_component(SHADER_SOURCE_PARSER_FOLDER ${SHADER_SOURCE_PARSER_HEADER} DIRECTORY)
get_filename_component(SHADER_SOURCE_STUPID_INCLUDE_FOLDER ${SHADER_SOURCE_PARSER_FOLDER} DIRECTORY)


# add_subdirectory(glt_parser)

# TODO: check if glad was generated with EXPORT
find_package(glad REQUIRED)
message(STATUS "glad found: ${glad_FOUND}")
find_package(OpenGL REQUIRED)
message(STATUS "OpenGL found: ${OpenGL_FOUND}")
message(STATUS ${OPENGL_LIBRARIES})

find_package(glm REQUIRED)
message(STATUS "glm found: ${glm_FOUND}")

# TODO: header-only library?
add_library(${PROJECT_NAME}
	OBJECT
		# basic types
		include/enums.hpp
		include/gltHandle.hpp
		include/glslt_traits.hpp
		include/type_converions.hpp
		include/basic_types.hpp
		##############
		
		${SHADER_SOURCE_PARSER_HEADER}

		include/shader_traits.hpp
		include/uniform_traits.hpp
		include/vao_traits.hpp
		include/program_traits.hpp
		
		include/gl_traits.hpp
		
		include/traits.hpp
		

		src/gl_traits.cpp
	)
	
target_link_libraries(${PROJECT_NAME}
	PUBLIC
		glm
		glad::glad
		
		${SHADER_SOURCE_PARSER}
	PRIVATE
		${OPENGL_LIBRARIES}
	)
	
# add INTERFACE_INCLUDE_DIRECTORIES
target_include_directories(${PROJECT_NAME}
	PUBLIC
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
		
		$<BUILD_INTERFACE:${SHADER_SOURCE_PARSER_FOLDER}> # this is a workaround. 
		$<BUILD_INTERFACE:${SHADER_SOURCE_STUPID_INCLUDE_FOLDER}> # even more stupid workaround!
	PRIVATE
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
	)
	
add_subdirectory(sandbox)



	




