# copy assets for tests and examples
cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

add_custom_target(CREATE_DIRECTORIES
	COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
	COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}/include
	)

add_custom_target(COPY_SHADERS
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/shaders ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders
	DEPENDS CREATE_DIRECTORIES
	)
	
find_package(glt_parser REQUIRED)

add_custom_target(PARSE_SHADERS
	COMMAND $<TARGET_FILE:glt_parser>
		-s ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders
		-d ${PROJECT_BINARY_DIR}/include
		--vert ".vs .vshader"
		--frag ".fs .fshader"
		--geom ".gs .gshader"
		--comp ".cs .cshader"
	DEPENDS COPY_SHADERS
	)
	
find_package(glfw3 REQUIRED)

add_custom_target(COPY_DEPENDENCIES
	COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:glfw> ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
	DEPENDS CREATE_DIRECTORIES
	)
	
if(${BUILD_EXAMPLES_ASSIMP})
	find_package(assimp REQUIRED)
	
	# this is shit (regarding managing assimp dependency)!
	add_custom_command(TARGET COPY_DEPENDENCIES POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:assimp::assimp> ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
		)
endif()

	
add_custom_target(COPY_RESOURCES
	COMMAND ${CMAKE_COMMAND} -E  copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/resources ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources 
	DEPENDS CREATE_DIRECTORIES
	)

set_target_properties(CREATE_DIRECTORIES COPY_SHADERS PARSE_SHADERS COPY_DEPENDENCIES COPY_RESOURCES
	PROPERTIES
		FOLDER assets
	)