cmake_minimum_required(VERSION 3.12 FATAL_ERROR)


# sandbox
find_package(glfw3 REQUIRED)
message(STATUS "glfw3 found: ${glfw3_FOUND}")

find_package(PlatformPath REQUIRED)
message(STATUS "PlatformPath found: ${PlatformPath_FOUND}")

add_library(gl_sandbox
	STATIC
		$<TARGET_OBJECTS:gl_traits>
		helpers.hpp
		helpers.cpp
	)
	
target_link_libraries(gl_sandbox
	PUBLIC
		gl_traits
		glfw
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
	"traits_equivalence"
	"buffer_fetch_attributes"
	"handles_tests"
	"BufferMap_tests"	
	"1.Batched_2_dif_VBO" 	# ok
	"2.Batched_1_one_VBO"	# ok
	"3.Compound_one_VBO"	# ok
	"4.Batch_Comp_one_VBO"	# ok
	"5.Batch_Comp_two_VBO"	# ok
	
	
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