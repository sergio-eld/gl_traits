cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

# sandbox
find_package(glfw3 REQUIRED)
message(STATUS "glfw3 found: ${glfw3_FOUND}")

# TODO: remove PlatformPath from here! Get path from command line input
#find_package(PlatformPath REQUIRED)
#message(STATUS "PlatformPath found: ${PlatformPath_FOUND}")

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
		${CMAKE_CURRENT_BINARY_DIR}/include # created via add_custom_command
	)
	
# VBO cases

set(TEST_CASES
	"traits_equivalence"
		
	"shaders_test" # test compile various shaders
	
	"program_uniforms_test" # test program and uniforms
	
	"handles_tests"
	"sequence_test"
	"buffer_test"
	"textures_test"
		
	"0.Raw_Batched_2_dif_VBO"
	
	"1.Batched_2_dif_VBO" 	# ok
	"2.Batched_1_one_VBO"	# ok
	"3.Compound_one_VBO"	# ok
	"4.Batch_Comp_one_VBO"	# ok
	
	"5.Render_texture_bg"
		
)

foreach(USE_CASE ${TEST_CASES})
	add_executable(${USE_CASE}
		${USE_CASE}.cpp
	)
	
target_link_libraries(${USE_CASE}
	PRIVATE
		gl_sandbox
#		PlatformPath
	)
endforeach()


add_custom_command(TARGET gl_sandbox POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E  copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/shaders 
		$<TARGET_PROPERTY:gl_sandbox,RUNTIME_OUTPUT_DIRECTORY>/$(ConfigurationName)
	COMMAND ${CMAKE_COMMAND} -E  copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/resources $<TARGET_PROPERTY:gl_sandbox,RUNTIME_OUTPUT_DIRECTORY>/$(ConfigurationName)/resources

	COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:glfw> $<TARGET_PROPERTY:gl_sandbox,RUNTIME_OUTPUT_DIRECTORY>/$(ConfigurationName)
	
	COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/include
	
	COMMAND $<TARGET_FILE:glt_parser> # ${GLT_GENERATOR} 
		-s ${CMAKE_CURRENT_SOURCE_DIR}/shaders 
		-d ${CMAKE_CURRENT_BINARY_DIR}/include
		--vert ".vs .vshader"
		--frag ".fs .fshader"
		--geom ".gs .gshader"
		--comp ".cs .cshader"
	)	