cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(gl_traits_sbuild LANGUAGES CXX)

#set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED True)

set(BUILD_TESTS ON CACHE BOOL "Build and run tests and examples for gl_traits")

set_property(DIRECTORY PROPERTY EP_BASE ${CMAKE_BINARY_DIR}/subprojects)
include(ExternalProject)

find_package(glt_parser QUIET)
message(STATUS "glt_parser found: ${glt_parser_FOUND}")

if(NOT ${glt_parser_FOUND})

	ExternalProject_Add(glt_parser_external
		SOURCE_DIR
			${PROJECT_SOURCE_DIR}/glt_parser
		CMAKE_ARGS
			-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
			#-DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
			#-DCMAKE_CXX_STANDARD_REQUIRED=${CMAKE_CXX_STANDARD_REQUIRED}
			-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
			
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
	
	set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${glt_parser_DIR}")
	
endif()

find_package(glad QUIET)
message(STATUS "glad found: ${glad_FOUND}")

if(NOT ${glad_FOUND})

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
	
	ExternalProject_Get_Property(glad_external INSTALL_DIR)
	message(STATUS "glad will be installed to: ${glad_DIR}")
	set(glad_DIR "${INSTALL_DIR}/lib/cmake/glad/")	
	set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${glad_DIR}")
	
endif() 

find_package(glm QUIET)
message(STATUS "glm found: ${glm_FOUND}")

if(NOT ${glm_FOUND})

	ExternalProject_Add(glm_external
		GIT_REPOSITORY
			https://github.com/g-truc/glm.git
		GIT_TAG
			0.9.9.5
		CMAKE_ARGS
			-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
			#-DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
			#-DCMAKE_CXX_STANDARD_REQUIRED=${CMAKE_CXX_STANDARD_REQUIRED}
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
		
	ExternalProject_Get_Property(glm_external INSTALL_DIR)
	message(STATUS "glm will be installed to: ${glm_DIR}")
	set(glm_DIR "${INSTALL_DIR}/lib/cmake/glm/")
	set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${glm_DIR}")
	
endif()

if(${BUILD_TESTS})

	find_package(glfw3 QUIET)
	message(STATUS "glfw found: ${glfw3_FOUND}")

	# Download and install external dependencies if not found
	if(NOT ${glfw3_FOUND})

		ExternalProject_Add(glfw_external
			GIT_REPOSITORY
				https://github.com/glfw/glfw.git
			CMAKE_ARGS
				-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
				#-DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
				#-DCMAKE_CXX_STANDARD_REQUIRED=${CMAKE_CXX_STANDARD_REQUIRED}
				
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
		
		ExternalProject_Get_Property(glfw_external INSTALL_DIR)
		message(STATUS "glfw will be installed to: ${glfw3_DIR}")
		set(glfw3_DIR "${INSTALL_DIR}/lib/cmake/glfw3/")
		set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${glfw3_DIR}")

	endif()

	find_package(assimp)
	message(STATUS "assimp found: ${assimp_FOUND}")
	
	if(NOT ${assimp_FOUND})

		ExternalProject_Add(assimp_external
			GIT_REPOSITORY
				https://github.com/assimp/assimp.git
			GIT_TAG
				v.5.0.0.rc2
			CMAKE_ARGS
				-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
				#-DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
				#-DCMAKE_CXX_STANDARD_REQUIRED=${CMAKE_CXX_STANDARD_REQUIRED}
				-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
				
				-DCMAKE_C_COMPILER_WORKS=1
				-DCMAKE_CXX_COMPILER_WORKS=1
				
				-DASSIMP_BUILD_TESTS=False
				-DASSIMP_BUILD_ASSIMP_TOOLS=False
				-DHUNTER_STATUS_PRINT=False
				-DHUNTER_TLS_VERIFY=False
				
			CMAKE_CACHE_ARGS
				-DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
			BUILD_COMMAND
				""
			#INSTALL_COMMAND
				#${CMAKE_COMMAND} --build . --target install --config Release
			BUILD_ALWAYS
				True
			)
		
		ExternalProject_Get_Property(assimp_external INSTALL_DIR)
		message(STATUS "Assimp will be installed to: ${INSTALL_DIR}")
		set(assimp_DIR "${INSTALL_DIR}/lib/cmake/assimp-5.0/") # TODO: modify not to use explicit version suffix
		set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${assimp_DIR}")

	endif()

endif()

ExternalProject_Add(gl_traits_src
	SOURCE_DIR
		${PROJECT_SOURCE_DIR}/gl_traits
	CMAKE_ARGS
		-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
		-DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
		-DCMAKE_CXX_STANDARD_REQUIRED=${CMAKE_CXX_STANDARD_REQUIRED}
		-DCMAKE_CXX_COMPILER_WORKS=1

		-Dglt_parser_DIR=${glt_parser_DIR}
		-Dglm_DIR=${glm_DIR}
		-Dglad_DIR=${glad_DIR}
		-Dglfw3_DIR=${glfw3_DIR}
		-Dassimp_DIR=${assimp_DIR}
		
	CMAKE_CACHE_ARGS
		-DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
	#BUILD_COMMAND
	#	""
	INSTALL_COMMAND
		""
	BUILD_ALWAYS
		True
	)
	
ExternalProject_Add_StepDependencies(gl_traits_src 
	build 
		glt_parser_external 
		glm_external 
		glad_external 
	)
	
if(${BUILD_TESTS})
	ExternalProject_Add_StepDependencies(gl_traits_src 
		build 
			glfw_external
			assimp_external
		)
endif()