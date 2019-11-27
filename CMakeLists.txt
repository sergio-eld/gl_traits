cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(gl_traits LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Substitute with find_package?
add_subdirectory(third_party/pod_reflection)
#add_subdirectory(third_party/dhconstexpr_lib)


# TODO: check if glad was generated with EXPORT
find_package(glad REQUIRED)
message(STATUS "glad found: ${glad_FOUND}")
find_package(OpenGL REQUIRED)
message(STATUS "OpenGL found: ${OpenGL_FOUND}")
message(STATUS ${OPENGL_LIBRARIES})

find_package(glm REQUIRED)
message(STATUS "glm found: ${glm_FOUND}")

# TODO: header-only library?
add_library(${PROJECT_NAME}
	OBJECT
		include/gl_traits.hpp
		
		include/gltEnums.hpp
		include/traits.hpp
		include/gltHandle.hpp
		
		# include/traits_common.hpp
		# include/buffer_traits.hpp
		# include/texture_traits.hpp
		# include/uniform_traits.hpp
		# include/glt_definitions.hpp

		src/gl_traits.cpp

		# src/shader_traits.cpp
	)
	
target_link_libraries(${PROJECT_NAME}
	PUBLIC
		# pod_reflection
		#dhconstexpr_lib
		
		glm
		glad::glad

	PRIVATE
		${OPENGL_LIBRARIES}
	)
	
# add INTERFACE_INCLUDE_DIRECTORIES
target_include_directories(${PROJECT_NAME}
	PUBLIC
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
	PRIVATE
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
	)
	
add_subdirectory(sandbox)



	




