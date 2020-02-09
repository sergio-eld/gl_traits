cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(gl_traits VERSION 0.1.1 LANGUAGES CXX) # https://semver.org/ how to auto-increment utilizing git commands?

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# TODO: this is for examples and tests
foreach(CONFIG_TYPE ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${CONFIG_TYPE} OUTPUTCONFIG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
endforeach()

message (STATUS "gl_tratis will be installed to ${CMAKE_INSTALL_PREFIX}")

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

message(STATUS "Build type set to ${CMAKE_BUILD_TYPE}")

include(GNUInstallDirs)

# revisit this
#set(CMAKE_ARCHIVE_OUPTUT_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
#set(CMAKE_LIBRARY_OUPTUT_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
#set(CMAKE_RUNTIME_OUPTUT_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})

set(INSTALL_LIBDIR ${CMAKE_INSTALL_LIBDIR} CACHE PATH "Installation directory for libraries")
set(INSTALL_BINDIR ${CMAKE_INSTALL_BINDIR} CACHE PATH "Installation directory for executables")
set(INSTALL_INCLUDEDIR ${CMAKE_INSTALL_INCLUDEDIR} CACHE PATH "Installation directory for headers")
if(WIN32 AND NOT CYGWIN)
	set(DEF_INSTALL_CMAKEDIR cmake/${PROJECT_NAME})
else()
	set(DEF_INSTALL_CMAKEDIR share/cmake/${PROJECT_NAME})
endif()
set(INSTALL_CMAKEDIR ${DEF_INSTALL_CMAKEDIR} CACHE PATH "Installation directory for cmake module")

foreach(p LIB BIN INCLUDE CMAKE)
	file(TO_NATIVE_PATH "${CMAKE_INSTALL_PREFIX}/${INSTALL_${p}DIR}" _path)
	message(STATUS "Installing ${p} components to ${_path}")
	unset(_path)
endforeach()

set(BUILD_TESTS OFF CACHE BOOL "Build general test")
set(BUILD_EXAMPLES OFF CACHE BOOL "Enable to build examples")
set(BUILD_EXAMPLES_ASSIMP OFF CACHE BOOL "Enable to build examples with assimp models loading")

add_subdirectory(src)
add_subdirectory(Helpers)

if (${BUILD_TESTS} OR ${BUILD_EXAMPLES})
	add_subdirectory(test_assets)
endif()

if(${BUILD_TESTS})
	enable_testing()
	add_subdirectory(tests)
endif()

if(${BUILD_EXAMPLES})
	add_subdirectory(examples)
endif()

if(${BUILD_EXAMPLES_ASSIMP})
	add_subdirectory(examples_assimp)
endif()




	




