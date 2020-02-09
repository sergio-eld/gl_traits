# build assimp examples here
cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

# this is shit (regarding managing assimp dependency)!
find_package(assimp REQUIRED)
string(FIND ${ASSIMP_LIBRARY_DIRS}/${ASSIMP_LIBRARIES} ".dll" EXT_FOUND)
string(SUBSTRING ${ASSIMP_LIBRARY_DIRS}/${ASSIMP_LIBRARIES} 0 ${EXT_FOUND} assimp_lib)

add_library(AssimpModel
		STATIC
			assimp_model_impl.h
			assimp_model_impl.cpp
			assimp_model.h
			assimp_model.cpp
		)
		
# assimp model library depends on a shader type info
add_dependencies(AssimpModel PARSE_SHADERS)	

# assimp model depends on loading Images (bad)
target_link_libraries(AssimpModel
	PUBLIC 	# TODO: make private ?
		gl_traits
		Helpers
		${assimp_lib}.lib
		#assimp::assimp # yields .dll instead of .lib 
	)

target_include_directories(AssimpModel
	PUBLIC 	# TODO: make private ?
		${ASSIMP_INCLUDE_DIRS}
		${PROJECT_BINARY_DIR}/include
	)

set_target_properties(AssimpModel
	PROPERTIES
		DEBUG_POSTFIX "_d"
	)

set(ASSIMP_EXAMPLES
	nanosuit_simple
	)
	
foreach(ex ${ASSIMP_EXAMPLES})
	add_executable(${ex}
			${ex}.cpp 
		)
	
	target_link_libraries(${ex}
		PRIVATE
			Helpers
			AssimpModel
		) 
	
	target_include_directories(${ex}
		PRIVATE
			${PROJECT_BINARY_DIR}/include
		)
	
	set_target_properties(${ex}
		PROPERTIES
			DEBUG_POSTFIX "_d"
			FOLDER examples_assimp
		)
	
	add_dependencies(${ex} COPY_DEPENDENCIES PARSE_SHADERS)
	
endforeach()