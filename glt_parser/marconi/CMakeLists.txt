cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

project(marconi LANGUAGES CXX)

add_executable(app
	app.cpp
	)
	
set(ARGUMENTS
	"-S \"C:/My Sources/marconi\" -B \"C:/builds/folder name\" -A x64"
	)	

set_target_properties(app
	PROPERTIES
		CXX_STANDARD 17
		CXX_STANDARD_REQUIRED True
		VS_DEBUGGER_COMMAND_ARGUMENTS ${ARGUMENTS}
	)
	
enable_testing()

add_test(
	Name TestValid1
	COMMAND 
		$<TARGET_FILE:app> 
	)