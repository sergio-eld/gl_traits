cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(glfw3 REQUIRED)
message(STATUS "glfw3 found: ${glfw3_FOUND}")

add_library(Helpers
	STATIC
		helpers.h
		helpers.cpp
	)
	
target_link_libraries(Helpers
	PUBLIC
		glfw
	)
	
target_include_directories(Helpers
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
	)
	
set_target_properties(Helpers
	PROPERTIES
		DEBUG_POSTFIX "_d"
	)
	