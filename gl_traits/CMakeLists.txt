cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(gl_traits LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)


find_package(OpenGL REQUIRED)
message(STATUS "OpenGL found: ${OpenGL_FOUND}")
message(STATUS ${OPENGL_LIBRARIES})

find_package(glad REQUIRED)
message(STATUS "glad found: ${glad_FOUND}")

find_package(glm REQUIRED)
message(STATUS "glm found: ${glm_FOUND}")

find_package(glt_parser QUIET)
message(STATUS "glt_parser found: ${glt_parser_FOUND}")

# this is for tests
find_package(glfw3 QUIET)
message(STATUS "glfw found: ${glfw3_FOUND}")


# TODO: header-only library?
add_library(${PROJECT_NAME}
	OBJECT
		# basic types
		include/enums.hpp
		include/gltHandle.hpp
		include/glslt_traits.hpp
		include/type_converions.hpp
		include/basic_types.hpp
		include/equivalence.hpp
		##############
		
		#${SHADER_SOURCE_PARSER_HEADER}
		include/sequence_layout.hpp
		include/Sequence.hpp

		include/buffer_traits.hpp
		include/shader_traits.hpp
		include/uniform_traits.hpp
		include/vao_traits.hpp
		include/program_traits.hpp
		include/texture_traits.hpp
		
		include/Texture.hpp
		
		include/gl_traits.hpp
			
		src/gl_traits.cpp
	)
	

target_link_libraries(${PROJECT_NAME}
	PUBLIC
		glm
		glad::glad
		
		#${SHADER_SOURCE_PARSER}
		#ParseAlgorithmPublic
	PRIVATE
		${OPENGL_LIBRARIES}
	)
	

# add INTERFACE_INCLUDE_DIRECTORIES
target_include_directories(${PROJECT_NAME}
	PUBLIC
		$<INSTALL_INTERFACE:include>
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
		
		#$<BUILD_INTERFACE:${SHADER_SOURCE_PARSER_FOLDER}> # this is a workaround. 
		#$<BUILD_INTERFACE:${SHADER_SOURCE_STUPID_INCLUDE_FOLDER}> # even more stupid workaround!
	PRIVATE
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
	)
	
add_subdirectory(sandbox)



	




