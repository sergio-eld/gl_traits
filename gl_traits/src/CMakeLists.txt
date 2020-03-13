cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(OpenGL REQUIRED)
message(STATUS "OpenGL found: ${OpenGL_FOUND}")
message(STATUS ${OPENGL_LIBRARIES})

find_package(glad REQUIRED)
message(STATUS "glad found: ${glad_FOUND}")

find_package(glm REQUIRED)
message(STATUS "glm found: ${glm_FOUND}")

set(PUBLIC_HEADERS 
		# basic types
		include/${PROJECT_NAME}/enums.hpp
		include/${PROJECT_NAME}/gltHandle.hpp
		include/${PROJECT_NAME}/glslt_traits.hpp
		include/${PROJECT_NAME}/type_converions.hpp
		include/${PROJECT_NAME}/basic_types.hpp
		include/${PROJECT_NAME}/equivalence.hpp
		##############
		
		#${SHADER_SOURCE_PARSER_HEADER}
		include/${PROJECT_NAME}/sequence_layout.hpp
		include/${PROJECT_NAME}/Sequence.hpp

		include/${PROJECT_NAME}/buffer_traits.hpp
		include/${PROJECT_NAME}/shader_traits.hpp
		include/${PROJECT_NAME}/uniform_traits.hpp
		include/${PROJECT_NAME}/vao_traits.hpp
		include/${PROJECT_NAME}/program_traits.hpp
		include/${PROJECT_NAME}/texture_traits.hpp
		
		include/${PROJECT_NAME}/Texture.hpp
		
		include/${PROJECT_NAME}/gl_traits.hpp
	)

add_library(${PROJECT_NAME}
	STATIC
		${PUBLIC_HEADERS}
		
		src/gl_traits.cpp
	)
	
set_target_properties(${PROJECT_NAME}
	PROPERTIES
		
		OUTPUT_NAME ${PROJECT_NAME}
		DEBUG_POSTFIX "_d"
		
	)

target_link_libraries(${PROJECT_NAME}
	PUBLIC
		glm
		glad::glad
		
	PRIVATE
		${OPENGL_LIBRARIES}
	)
	
target_include_directories(${PROJECT_NAME}
	PUBLIC
		$<INSTALL_INTERFACE:${INSTALL_INCLUDEDIR}/${PROJECT_NAME}>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}>
		
	PRIVATE
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
	)
	
set(targets_export_name ${PROJECT_NAME}Targets)
set(project_config ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake)
set(version_config ${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake)
	
include(CMakePackageConfigHelpers)

write_basic_package_version_file(
		${version_config}
	VERSION
		${PROJECT_VERSION}
	COMPATIBILITY SameMajorVersion
	)

configure_package_config_file(
		${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in
		${project_config}
	INSTALL_DESTINATION
		${INSTALL_CMAKEDIR}
	)

install(TARGETS
		${PROJECT_NAME}
	EXPORT
		${targets_export_name}
	ARCHIVE
		DESTINATION ${INSTALL_LIBDIR}
		COMPONENT lib
	LIBRARY
		DESTINATION ${INSTALL_LIBDIR}
		COMPONENT lib
	RUNTIME
		DESTINATION ${INSTALL_BINDIR}
		COMPONENT bin
	)

install(EXPORT
		${targets_export_name}
	DESTINATION
		${INSTALL_CMAKEDIR}
	)
	
install(FILES ${PUBLIC_HEADERS} DESTINATION ${INSTALL_INCLUDEDIR}/${PROJECT_NAME} COMPONENT dev)
install(FILES 
		${project_config}
		${version_config}
	DESTINATION
		${INSTALL_CMAKEDIR}
	)