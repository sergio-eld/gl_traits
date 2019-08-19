cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(NOT hasParent)
set(ENABLE_EXAMPLE ON)
else()
set(ENABLE_EXAMPLE OFF)
endif()

message(STATUS ${ENABLE_EXAMPLE})

option(BUILD_EXAMPLE "Compile example?" ENABLE_EXAMPLE)

add_library(dhconstexpr_lib INTERFACE)

target_include_directories(dhconstexpr_lib INTERFACE include/)

get_directory_property(hasParent PARENT_DIRECTORY)
message(STATUS "parent directory:" ${hasParent})


if(BUILD_EXAMPLE)
message(STATUS "Building example")
add_executable(constexpr_lib_example
	example/main.cpp
	)

target_link_libraries(constexpr_lib_example
	PRIVATE
	dhconstexpr_lib
	)
endif(BUILD_EXAMPLE)