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
		RUNTIME_OUTPUT_DIRECTORY bin
		VS_DEBUGGER_COMMAND_ARGUMENTS ${ARGUMENTS}
		
	)
	
enable_testing()

add_test(NAME TesValid1
	COMMAND 
		$<TARGET_FILE:app> 
		-S "C:/My Sources/marconi" 
		-B "C:/builds/folder name" 
		-A x64
	)
	
add_test(NAME TesValid2
	COMMAND 
		$<TARGET_FILE:app> 
		-S "C:/My Sources/marconi" 
		-B "C:/builds/folder name" 
	)
	
add_test(NAME TesValid3
	COMMAND 
		$<TARGET_FILE:app> 
		-B "C:/builds/folder name" 
		-S "C:/My Sources/marconi" 
	)