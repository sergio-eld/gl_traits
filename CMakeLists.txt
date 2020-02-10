cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(gl_traits_sbuild LANGUAGES CXX)


set(BUILD_TESTS OFF CACHE BOOL "Build general test")
set(BUILD_EXAMPLES OFF CACHE BOOL "Enable to build examples")
set(BUILD_EXAMPLES_ASSIMP OFF CACHE BOOL "Enable to build examples with assimp models loading")

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

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
	set(glt_parser_DIR "${INSTALL_DIR}/lib/cmake/glt_parser/" CACHE PATH "" FORCE)
	
	set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${glt_parser_DIR}")
	
endif()

find_package(glad QUIET)
message(STATUS "glad found: ${glad_FOUND}")
if (${glad_FOUND})
	add_custom_target(glad_external)
else()
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
	set(glad_DIR "${INSTALL_DIR}/lib/cmake/glad/" CACHE PATH "" FORCE)	
	set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${glad_DIR}")
	
endif() 

find_package(glm QUIET)
message(STATUS "glm found: ${glm_FOUND}")

if(${glm_FOUND})
	add_custom_target(glm_external)
else()
	ExternalProject_Add(glm_external
		GIT_REPOSITORY
			https://github.com/g-truc/glm.git
		GIT_TAG
			0.9.9.5
		CMAKE_ARGS
			-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
			
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
	set(glm_DIR "${INSTALL_DIR}/lib/cmake/glm/" CACHE PATH "" FORCE)
	set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${glm_DIR}")
	
endif()

if(${BUILD_TESTS})

	find_package(glfw3 QUIET)
	message(STATUS "glfw found: ${glfw3_FOUND}")

	# Download and install external dependencies if not found
	if(${glfw3_FOUND})
		add_custom_target(glfw_external)
	else()
		ExternalProject_Add(glfw_external
			GIT_REPOSITORY
				https://github.com/glfw/glfw.git
			CMAKE_ARGS
				-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
				
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
		set(glfw3_DIR "${INSTALL_DIR}/lib/cmake/glfw3/" CACHE PATH "" FORCE)
		set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${glfw3_DIR}")

	endif()

	if (${BUILD_EXAMPLES_ASSIMP})
		find_package(assimp QUIET)
		message(STATUS "assimp found: ${assimp_FOUND}")
		
		if(${assimp_FOUND})
			add_custom_target(assimp_external)
		else()
			ExternalProject_Add(assimp_external
				GIT_REPOSITORY
					https://github.com/assimp/assimp.git
				GIT_TAG
					v5.0.1
				CMAKE_ARGS
					-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
					-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
					
					-DCMAKE_C_COMPILER_WORKS=1
					-DCMAKE_CXX_COMPILER_WORKS=1
					
					-DASSIMP_BUILD_TESTS=False
					-DASSIMP_BUILD_ASSIMP_TOOLS=False
					-DHUNTER_STATUS_PRINT=False
					-DHUNTER_TLS_VERIFY=False
					
					-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}

				CMAKE_CACHE_ARGS
					-DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
				BUILD_COMMAND
					""
				#INSTALL_COMMAND
				#	${CMAKE_COMMAND} --build . --target install --config ${CMAKE_BUILD_TYPE}
				#BUILD_ALWAYS
				#	True
				)
			
			ExternalProject_Get_Property(assimp_external INSTALL_DIR)
			message(STATUS "Assimp will be installed to: ${INSTALL_DIR}")
			set(assimp_DIR "${INSTALL_DIR}/lib/cmake/assimp-5.0/" CACHE PATH "" FORCE) # TODO: modify not to use explicit version suffix
			set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${assimp_DIR}")

		endif()
	endif(${BUILD_EXAMPLES_ASSIMP})
	
endif()

# TODO: add install dir and fetch dependencies on installation
ExternalProject_Add(gl_traits_src
	SOURCE_DIR
		${PROJECT_SOURCE_DIR}/gl_traits
	BINARY_DIR
		${PROJECT_BINARY_DIR}/gl_traits
	#INSTALL_DIR 
		#${PROJECT_BINARY_DIR}/gl_traits/install
	CMAKE_ARGS
		-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
		
		-DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}

		-DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
		-DCMAKE_CXX_STANDARD_REQUIRED=${CMAKE_CXX_STANDARD_REQUIRED}
		-DCMAKE_CXX_COMPILER_WORKS=1

		-Dglt_parser_DIR=${glt_parser_DIR}
		-Dglm_DIR=${glm_DIR}
		-Dglad_DIR=${glad_DIR}
		-Dglfw3_DIR=${glfw3_DIR}
		-Dassimp_DIR=${assimp_DIR}
		
		-DBUILD_TESTS=${BUILD_TESTS}
		-DBUILD_EXAMPLES=${BUILD_EXAMPLES}
		-DBUILD_EXAMPLES_ASSIMP=${BUILD_EXAMPLES_ASSIMP}
		
		-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
		
	CMAKE_CACHE_ARGS
		-DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
	BUILD_COMMAND
	""#	${CMAKE_COMMAND} --build . --target ALL_BUILD --config Release
	INSTALL_COMMAND
		${CMAKE_COMMAND} --build . --target INSTALL --config Release
	#BUILD_ALWAYS
	#	True
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
			)
			
	ExternalProject_Add_Step(gl_traits_src tests
		COMMENT "Running general tests for gl_traits..."
		COMMAND ${CMAKE_COMMAND} --build . --target RUN_TESTS --config Release
		ALWAYS True
		EXCLUDE_FROM_MAIN True
		)
	
	# this #%$#%^# does not work!!!! "--build . --target gl_traits_src-tests" yields an error: projec file does not exist
	ExternalProject_Add_StepTargets(gl_traits_src tests)
endif()

if(${BUILD_EXAMPLES_ASSIMP})
	ExternalProject_Add_StepDependencies(gl_traits_src 
		build 
			assimp_external
		)
endif(${BUILD_EXAMPLES_ASSIMP})
