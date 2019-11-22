cmake_minimum_required(VERSION 3.12 FATAL_ERROR)


# sandbox
find_package(glfw3 REQUIRED)
message(STATUS "glfw3 found: ${glfw3_FOUND}")

find_package(PlatformPath REQUIRED)
message(STATUS "PlatformPath found: ${PlatformPath_FOUND}")

add_library(gl_sandbox
	STATIC
		$<TARGET_OBJECTS:gl_traits>
		# assertions.cpp
		helpers.hpp
		helpers.cpp
	)
	
target_link_libraries(gl_sandbox
	PUBLIC
		gl_traits
		glfw
		#PlatformPath
	)
	
target_include_directories(gl_sandbox
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
	)
	
# VBO cases

set(VBO_USE_CASES
	"traits_common"
	"traits_attribute_compound"
	"traits_attribute_nth_attribute"
	"equivalence"
	"handles_tests"
	"BufferMap_tests"
	#"1.Batched_2_dif_VBO" 
	#"2.Batched_2_one_VBO" 
	#"3.Compound_one_VBO"
)

foreach(USE_CASE ${VBO_USE_CASES})
	add_executable(${USE_CASE}
		${USE_CASE}.cpp
	)
	
target_link_libraries(${USE_CASE}
	PRIVATE
		gl_sandbox
		PlatformPath
	)
endforeach()
	

add_custom_command(TARGET gl_sandbox PRE_BUILD
	COMMAND ${CMAKE_COMMAND} -E  copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/shaders 
		$<TARGET_PROPERTY:gl_sandbox,RUNTIME_OUTPUT_DIRECTORY>/$(ConfigurationName)
	COMMAND ${CMAKE_COMMAND} -E  copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/resources $<TARGET_PROPERTY:gl_sandbox,RUNTIME_OUTPUT_DIRECTORY>/$(ConfigurationName)/resources

	COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:glfw> $<TARGET_PROPERTY:gl_sandbox,RUNTIME_OUTPUT_DIRECTORY>/$(ConfigurationName)
	)	