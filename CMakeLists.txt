cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(gl_traits LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/glt_parser)
execute_process(COMMAND ${CMAKE_COMMAND} -G ${CMAKE_GENERATOR}
					-S ${CMAKE_SOURCE_DIR}/glt_parser -B ${CMAKE_BINARY_DIR}/glt_parser/build
					-A x64 -DCMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE=${CMAKE_BINARY_DIR}/glt_parser
					-DCMAKE_BUILD_TYPE=Release
				)
execute_process(COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/glt_parser/build --target glt_parser --config Release)


find_program(GLT_GENERATOR ${CMAKE_BINARY_DIR}/glt_parser/glt_parser.exe)


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
		include/gl_traits.hpp
		
		include/gltEnums.hpp
		include/traits.hpp
		include/gltHandle.hpp

		src/gl_traits.cpp
	)
	
target_link_libraries(${PROJECT_NAME}
	PUBLIC
	
		glm
		glad::glad

	PRIVATE
		${OPENGL_LIBRARIES}
	)
	
# add INTERFACE_INCLUDE_DIRECTORIES
target_include_directories(${PROJECT_NAME}
	PUBLIC
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
	PRIVATE
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
	)
	
add_subdirectory(sandbox)



	




