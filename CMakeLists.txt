cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(gl_traits LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Substitute with find_package?
add_subdirectory(third_party/pod_reflection)
add_subdirectory(third_party/dhconstexpr_lib)


# TODO: check if glad was generated with EXPORT
find_package(glad REQUIRED)
message(STATUS "glad found: ${glad_FOUND}")
find_package(OpenGL REQUIRED)
message(STATUS "OpenGL found: ${OpenGL_FOUND}")
message(STATUS ${OPENGL_LIBRARIES})

find_package(glm REQUIRED)
message(STATUS "glm found: ${glm_FOUND}")

add_library(${PROJECT_NAME}
	OBJECT
		include/gl_traits.hpp
		include/traits_common.hpp
		include/buffer_traits.hpp
		include/texture_traits.hpp
		include/uniform_traits.hpp
		include/glt_definitions.hpp

		src/gl_traits.cpp

		src/shader_traits.cpp
	)
	
target_link_libraries(${PROJECT_NAME}
	PUBLIC
		pod_reflection
		dhconstexpr_lib
		
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
	
# move to subdir
# sandbox
find_package(glfw3 REQUIRED)
message(STATUS "glfw3 found: ${glfw3_FOUND}")

find_package(PlatformPath REQUIRED)
message(STATUS "PlatformPath found: ${PlatformPath_FOUND}")

add_library(gl_sandbox
	STATIC
		$<TARGET_OBJECTS:gl_traits>
		sandbox/assertions.cpp
		sandbox/helpers.hpp
		sandbox/helpers.cpp
	)

target_compile_definitions(gl_sandbox
	PUBLIC
	#	GL_TRAITS_STATIC
	)	

target_link_libraries(gl_sandbox
	PUBLIC
		gl_traits
		glfw
		PlatformPath
	)
	
target_include_directories(gl_sandbox
	PUBLIC
		sandbox
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
	COMMAND ${CMAKE_COMMAND} -E  copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/sandbox/shaders $<TARGET_PROPERTY:gl_sandbox,RUNTIME_OUTPUT_DIRECTORY>
	COMMAND ${CMAKE_COMMAND} -E  copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/sandbox/resources $<TARGET_PROPERTY:gl_sandbox,RUNTIME_OUTPUT_DIRECTORY>/resources

	COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:glfw> $<TARGET_PROPERTY:gl_sandbox,RUNTIME_OUTPUT_DIRECTORY>
	)	