# run tests here
cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CTEST_PARALLEL_LEVEL 10)

set(TESTS
	"traits_equivalence"
	
	"shaders_test" # test compile various shaders
	"program_uniforms_test" # test program and uniforms
	
	"handles_tests"
	"sequence_test"
	"buffer_test"
	"textures_test"
	)

# inner tests
foreach(test ${TESTS})
	add_executable(${test}
			${test}.cpp
		)
	
	target_link_libraries(${test}
		PRIVATE
			gl_traits
			Helpers
		)
	
	target_include_directories(${test}
		PRIVATE
			${PROJECT_BINARY_DIR}/include
		)
	
	set_target_properties(${test}
		PROPERTIES
			DEBUG_POSTFIX "_d"
			FOLDER general_tests
		)
		
	add_dependencies(${test} COPY_SHADERS PARSE_SHADERS)
	
	add_test(
		NAME ${test}
		COMMAND $<TARGET_FILE:${test}>
		)
endforeach()
	