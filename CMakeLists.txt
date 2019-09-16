cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(gl_traits LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(gl_traits_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(third_party/pod_reflection)
add_subdirectory(third_party/dhconstexpr_lib)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")
set(GLM_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

find_package(OpenGL REQUIRED)
message(STATUS ${OPENGL_LIBRARIES})

find_package(GLM REQUIRED)
message(STATUS "GLM included at ${GLM_INCLUDE_DIR}")

add_library(${PROJECT_NAME}
	OBJECT
		${gl_traits_INCLUDE_DIR}/gl_traits.hpp
		${gl_traits_INCLUDE_DIR}/traits_common.hpp
		${gl_traits_INCLUDE_DIR}/buffer_traits.hpp
		${gl_traits_INCLUDE_DIR}/texture_traits.hpp
		${gl_traits_INCLUDE_DIR}/uniform_traits.hpp
		${gl_traits_INCLUDE_DIR}/glt_definitions.hpp


		#${gl_traits_INCLUDE_DIR}/glad/glad.h
		src/gl_traits.cpp
		src/glad.c

		src/shader_traits.cpp
	)
	
target_link_libraries(${PROJECT_NAME}
	PUBLIC
		pod_reflection
		dhconstexpr_lib
		${GLM}

	PRIVATE
		${OPENGL_LIBRARIES}
	)
	
target_include_directories(${PROJECT_NAME}
	PUBLIC
		${gl_traits_INCLUDE_DIR}
	PRIVATE
		/src
	)