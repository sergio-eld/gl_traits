cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

set(BUILD_ASSIMP_EXAMPLES OFF CACHE BOOL "Build examples using assimp library")

# sandbox
find_package(glfw3 REQUIRED)
message(STATUS "glfw3 found: ${glfw3_FOUND}")

find_package(glt_parser REQUIRED)

set(HELPER_LIBS $<TARGET_FILE:glfw>)

if (${BUILD_ASSIMP_EXAMPLES})
	find_package(assimp REQUIRED)
	message(STATUS "assimp found: ${assimp_FOUND}")
	
	list(APPEND HELPER_LIBS $<TARGET_FILE:assimp>)
endif()


# how to make this copy to bin/Release|Debug?
add_custom_target(COPY_DEPENDENCIES
	COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
	COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/include
	
	COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:glfw> ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
	COMMAND ${CMAKE_COMMAND} -E  copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/shaders 
		${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders
	COMMAND ${CMAKE_COMMAND} -E  copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/resources ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources 
	)
	
add_custom_target(PARSE_SHADERS
	COMMAND $<TARGET_FILE:glt_parser> 
		-s ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders 
		-d ${CMAKE_CURRENT_BINARY_DIR}/include
		--vert ".vs .vshader"
		--frag ".fs .fshader"
		--geom ".gs .gshader"
		--comp ".cs .cshader"
	)


if (${BUILD_ASSIMP_EXAMPLES})
#	add_custom_command(TARGET COPY_DEPENDENCIES POST_BUILD
#		COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:assimp> ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
#		)
endif()
# VBO cases

add_library(Helpers
	STATIC
		helpers.hpp
		helpers.cpp
	)
	
set_target_properties(Helpers
	PROPERTIES
		DEBUG_POSTFIX "_d"
	)
	
target_include_directories(Helpers
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/include # created via add_custom_command
	)
	
target_link_libraries(Helpers
	PUBLIC
		gl_traits
		#${HELPER_LIBS}
		glfw
	)

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
			Helpers
		)
	
	set_target_properties(${USE_CASE}
		PROPERTIES
			DEBUG_POSTFIX "_d"
		)

	add_dependencies(${USE_CASE} COPY_DEPENDENCIES PARSE_SHADERS)
	
endforeach()


#add_custom_command(TARGET gl_sandbox POST_BUILD
#	COMMAND ${CMAKE_COMMAND} -E  copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/shaders 
#		$<TARGET_PROPERTY:gl_sandbox,RUNTIME_OUTPUT_DIRECTORY>/$(ConfigurationName)
#	COMMAND ${CMAKE_COMMAND} -E  copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/resources $<TARGET_PROPERTY:gl_sandbox,RUNTIME_OUTPUT_DIRECTORY>/$(ConfigurationName)/resources

	
	
#	COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/include
	
#	
#	)	