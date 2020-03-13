#build examples here
cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

set(EXAMPLES
		
	"0.Raw_Batched_2_dif_VBO" # not working
	
	"1.Batched_2_dif_VBO" 	
	"2.Batched_1_one_VBO"	
	"3.Compound_one_VBO"	
	"4.Batch_Comp_one_VBO"	
	"5.Render_texture_bg"
)

foreach(ex ${EXAMPLES})
	add_executable(${ex}
		${ex}.cpp
	)
	
	target_link_libraries(${ex}
		PRIVATE
			gl_traits
			Helpers
		)
	
	target_include_directories(${ex}
		PRIVATE
			${PROJECT_BINARY_DIR}/include
		)
	
	set_target_properties(${ex}
		PROPERTIES
			DEBUG_POSTFIX "_d"
			FOLDER examples
		)

	add_dependencies(${ex} COPY_RESOURCES COPY_DEPENDENCIES PARSE_SHADERS)
	
endforeach()

