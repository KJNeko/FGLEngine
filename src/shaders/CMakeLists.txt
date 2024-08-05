


file(GLOB_RECURSE SHADERS
		"${CMAKE_CURRENT_SOURCE_DIR}/**.frag"
		"${CMAKE_CURRENT_SOURCE_DIR}/**.vert"
		"${CMAKE_CURRENT_SOURCE_DIR}/**.tesc"
		"${CMAKE_CURRENT_SOURCE_DIR}/**.tese"
		"${CMAKE_CURRENT_SOURCE_DIR}/**.glsl"
)

foreach (SHADER IN LISTS SHADERS)
	get_filename_component(FILENAME ${SHADER} NAME)
	get_filename_component(FILE_DIRECTORY ${SHADER} DIRECTORY)
	file(RELATIVE_PATH REL_PATH ${CMAKE_CURRENT_SOURCE_DIR} ${FILE_DIRECTORY})

	if (NOT REL_PATH STREQUAL "")
		set(REL_PATH ${REL_PATH}/)
		file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin/shaders/${REL_PATH})
	endif ()

	set(OUT_DIR ${CMAKE_BINARY_DIR}/bin/shaders/${REL_PATH})
	set(OUT_PATH ${OUT_DIR}${FILENAME})

	#add_custom_command(OUTPUT ${OUT_DIR}
	#		COMMAND ${CMAKE_COMMAND} -E make_directory ${OUT_DIR} COMMENT "Creating directory ${OUT_DIR}")

	#add_custom_command(OUTPUT ${OUT_PATH}
	#		COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${SHADER_DEBUG_FLAGS} ${SHADER} -o ${OUT_PATH} DEPENDS ${SHADER} ${OUT_DIR}
	#		COMMENT "Compiling ${SHADER} ")

	add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/bin/shaders/${FILENAME}
			COMMAND ${CMAKE_COMMAND} -E copy ${SHADER} ${OUT_PATH} DEPENDS ${SHADER}
			COMMENT "Copying ${ASSET} to ${OUT_PATH}")

	list(APPEND SPV_SHADERS ${CMAKE_BINARY_DIR}/bin/shaders/${FILENAME})
	list(APPEND ADDITIONAL_CLEAN_FILES ${OUT_PATH})
endforeach ()

add_custom_target(shaders ALL DEPENDS ${SPV_SHADERS})
