cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(gl_traits LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

set(GLM_SOURCE_DIR "" CACHE PATH "GLM source dir to build from")
set(GLM_SOURCE_DIR ${GLM_SOURCE_DIR})
set(GLAD_SOURCE_DIR "" CACHE PATH "GLAD source dir to build from")
set(GLAD_SOURCE_DIR ${GLAD_SOURCE_DIR})

set_property(DIRECTORY PROPERTY EP_BASE ${CMAKE_BINARY_DIR}/subprojects)
include(ExternalProject)

find_package(OpenGL REQUIRED)
message(STATUS "OpenGL found: ${OpenGL_FOUND}")
message(STATUS ${OPENGL_LIBRARIES})

find_package(glt_parser QUIET)
message(STATUS "glt_parser found: ${glt_parser_FOUND}")

find_package(glad QUIET)
message(STATUS "glad found: ${glad_FOUND}")

find_package(glm QUIET)
message(STATUS "glm found: ${glm_FOUND}")

find_package(glfw3 QUIET)
message(STATUS "glfw found: ${glfw3_FOUND}")


if(${glm_FOUND} AND ${glad_FOUND} AND ${glfw3_FOUND})
	add_custom_target(Rescan)
else()

	ExternalProject_Add(glt_parser_external
		SOURCE_DIR
			${PROJECT_SOURCE_DIR}/glt_parser
		CMAKE_ARGS
			-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
			-DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
			-DCMAKE_CXX_STANDARD_REQUIRED=${CMAKE_CXX_STANDARD_REQUIRED}
			-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
			
			-DCMAKE_CXX_COMPILER_WORKS=1
		CMAKE_CACHE_ARGS
			-DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
		BUILD_COMMAND
			""
		INSTALL_COMMAND
			"" #${CMAKE_COMMAND} --build . --target install --config Release
		BUILD_ALWAYS
			True
		)

	ExternalProject_Add(glm_external
		GIT_REPOSITORY
			https://github.com/g-truc/glm.git
		GIT_TAG
			0.9.9.5
		CMAKE_ARGS
			-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
			-DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
			-DCMAKE_CXX_STANDARD_REQUIRED=${CMAKE_CXX_STANDARD_REQUIRED}
			-DGLM_TEST_ENABLE=False
			-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
			
			-DCMAKE_CXX_COMPILER_WORKS=1
		CMAKE_CACHE_ARGS
			-DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
		BUILD_COMMAND
			""
		INSTALL_COMMAND
			""${CMAKE_COMMAND} --build . --target install --config Release
		BUILD_ALWAYS
			True
		)

	if (NOT ${GLAD_SOURCE_DIR})
		set(GLAD_SOURCE_DIR "${CMAKE_BINARY_DIR}/subprojects/Source/glad_external/")
	endif()

	ExternalProject_Add(glad_external
		GIT_REPOSITORY
			https://github.com/Dav1dde/glad.git
		CMAKE_ARGS
			-DGLAD_PROFILE="core"
			-DGLAD_API=""
			-DGLAD_GENERATOR="c"
			-DGLAD_SPEC=gl
			-DGLAD_REPRODUCIBLE="ON"
			-DGLAD_INSTALL="ON"
			
			-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
			
		CMAKE_CACHE_ARGS
			-DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
		BUILD_COMMAND
			""
		INSTALL_COMMAND
			${CMAKE_COMMAND} --build . --target install --config Release
		BUILD_ALWAYS
			True
		)

	ExternalProject_Add(glfw_external
		GIT_REPOSITORY
			https://github.com/glfw/glfw.git
		CMAKE_ARGS
			-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
			-DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
			-DCMAKE_CXX_STANDARD_REQUIRED=${CMAKE_CXX_STANDARD_REQUIRED}
			
			-DGLFW_BUILD_DOCS=False
			-DGLFW_BUILD_EXAMPLES=False
			-DGLFW_BUILD_TESTS=False
			-DGLFW_INSTALL=True
			
			-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
			
			-DCMAKE_C_COMPILER_WORKS=1
			-DCMAKE_CXX_COMPILER_WORKS=1
			
		CMAKE_CACHE_ARGS
			-DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
		BUILD_COMMAND
			""
		INSTALL_COMMAND
			${CMAKE_COMMAND} --build . --target install --config Release
		BUILD_ALWAYS
			True
		)

	ExternalProject_Get_Property(glt_parser_external INSTALL_DIR)
	set(glt_parser_DIR "${INSTALL_DIR}/lib/cmake/glt_parser/")

	ExternalProject_Get_Property(glm_external INSTALL_DIR)
	set(glm_DIR "${INSTALL_DIR}/lib/cmake/glm/")
	
	ExternalProject_Get_Property(glad_external INSTALL_DIR)
	set(glad_DIR "${INSTALL_DIR}/lib/cmake/glad/")
	
	ExternalProject_Get_Property(glfw_external INSTALL_DIR)
	set(glfw3_DIR "${INSTALL_DIR}/lib/cmake/glad/")

	set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${glt_parser_DIR};${glm_DIR};${glad_DIR};${glfw3_DIR}")

	message(STATUS "Dependend packages not found! Build Rescan or provide paths to installed dependencies.")
	add_custom_target(Rescan ${CMAKE_COMMAND} ${CMAKE_SOURCE_DIR} -DCMAKE_PREFIX_PATH:STRING="${CMAKE_PREFIX_PATH}" -Dglt_parser_DIR="${glt_parser_DIR}" -Dglm_DIR="${glm_DIR}" -Dglad_DIR="${glad_DIR}" -Dglfw3_DIR="${glfw3_DIR}" DEPENDS glt_parser_external glm_external glad_external glfw_external)
endif()


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

add_dependencies(${PROJECT_NAME} Rescan)	
	
if(${glad_FOUND} AND ${glm_FOUND})
	target_link_libraries(${PROJECT_NAME}
		PUBLIC
			glm
			glad::glad
			
			#${SHADER_SOURCE_PARSER}
			#ParseAlgorithmPublic
		PRIVATE
			${OPENGL_LIBRARIES}
		)
endif()		

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
	
#add_subdirectory(sandbox)



	




