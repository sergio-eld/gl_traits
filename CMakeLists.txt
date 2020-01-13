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
					-DCMAKE_INSTALL_PREFIX=${PROJECT_BINARY_DIR}/glt_parser
				)
				
execute_process(COMMAND ${CMAKE_COMMAND} --build ${PROJECT_BINARY_DIR}/glt_parser/build --target install  --config Debug -- /nologo /verbosity:minimal)
execute_process(COMMAND ${CMAKE_COMMAND} --build ${PROJECT_BINARY_DIR}/glt_parser/build --target install  --config Release -- /nologo /verbosity:minimal)
execute_process(COMMAND ${CMAKE_COMMAND} --build ${PROJECT_BINARY_DIR}/glt_parser/build --target install  --config RelWithDebInfo -- /nologo /verbosity:minimal)
execute_process(COMMAND ${CMAKE_COMMAND} --build ${PROJECT_BINARY_DIR}/glt_parser/build --target install  --config MinSizeRel -- /nologo /verbosity:minimal)

set(glt_parser_DIR ${PROJECT_BINARY_DIR}/glt_parser/lib/cmake/glt_parser)
find_package(glt_parser REQUIRED)

message(STATUS "glt parser found at ${gl_traits_FOUND}")
message(STATUS "ParserAlgorithmPublic found at ${ParseAlgorithmPublic_FOUND}")

add_custom_target(build_glt_parser
	COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}/glt_parser
	COMMAND ${CMAKE_COMMAND} -G ${CMAKE_GENERATOR}
					-S ${PROJECT_SOURCE_DIR}/glt_parser -B ${PROJECT_BINARY_DIR}/glt_parser/build
					-A x64 
					-DCMAKE_INSTALL_PREFIX=${PROJECT_BINARY_DIR}/glt_parser
				
	COMMAND ${CMAKE_COMMAND} --build ${PROJECT_BINARY_DIR}/glt_parser/build --target install  --config Debug -- /nologo /verbosity:minimal
	COMMAND ${CMAKE_COMMAND} --build ${PROJECT_BINARY_DIR}/glt_parser/build --target install  --config Release -- /nologo /verbosity:minimal
	COMMAND ${CMAKE_COMMAND} --build ${PROJECT_BINARY_DIR}/glt_parser/build --target install  --config RelWithDebInfo -- /nologo /verbosity:minimal
	COMMAND ${CMAKE_COMMAND} --build ${PROJECT_BINARY_DIR}/glt_parser/build --target install  --config MinSizeRel -- /nologo /verbosity:minimal
)

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
		
		#${SHADER_SOURCE_PARSER_HEADER}

		include/buffer_traits.hpp
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
		
		#${SHADER_SOURCE_PARSER}
		ParseAlgorithmPublic
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



	




