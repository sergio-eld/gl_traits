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
	
# sandbox
add_library(gl_sandbox
	STATIC
		$<TARGET_OBJECTS:gl_traits>
		sandbox/assertions.cpp
		sandbox/helpers.hpp
		sandbox/helpers.cpp
	)

target_link_directories(gl_sandbox
	PUBLIC
		sandbox/libs
	)

target_compile_definitions(gl_sandbox
	PUBLIC
	#	GL_TRAITS_STATIC
	)	

target_link_libraries(gl_sandbox
	PUBLIC
		gl_traits
		glfw3/glfw3dll
	)
	
target_include_directories(gl_sandbox
	PUBLIC
		sandbox
		glfw
	)

# VBO cases

set(VBO_USE_CASES
	"1.Batched_2_dif_VBO"
	"2.Batched_2_one_VBO"
	"3.Compound_one_VBO"
	#"4.Batched_Compound_2_dif_VBO"
	#"5.Batched_Compound_one_VBO" not possible. Compund must go first and be the only one
)

foreach(USE_CASE ${VBO_USE_CASES})
	add_executable(${USE_CASE}
		sandbox/${USE_CASE}.cpp
	)
	
target_link_libraries(${USE_CASE}
	PRIVATE
		gl_sandbox
	)
endforeach()
	

add_custom_command(TARGET gl_sandbox PRE_BUILD
	COMMAND ${CMAKE_COMMAND} -E  copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/sandbox/shaders $<TARGET_FILE_DIR:gl_sandbox>
	COMMAND ${CMAKE_COMMAND} -E  copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/sandbox/resources $<TARGET_FILE_DIR:gl_sandbox>/resources

	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/sandbox/libs/glfw3/glfw3.dll $<TARGET_FILE_DIR:gl_sandbox>
	)	