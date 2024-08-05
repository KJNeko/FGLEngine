file(GLOB_RECURSE ASSETS
		${CMAKE_CURRENT_SOURCE_DIR}/**.obj
		${CMAKE_CURRENT_SOURCE_DIR}/**.png
		${CMAKE_CURRENT_SOURCE_DIR}/**.jpg
		${CMAKE_CURRENT_SOURCE_DIR}/**.glb
		${CMAKE_CURRENT_SOURCE_DIR}/**.bin
		${CMAKE_CURRENT_SOURCE_DIR}/**.gltf
)

foreach (ASSET IN LISTS ASSETS)
	get_filename_component(FILENAME ${ASSET} NAME)
	get_filename_component(FILE_DIRECTORY ${ASSET} DIRECTORY)
	file(RELATIVE_PATH REL_PATH ${CMAKE_CURRENT_SOURCE_DIR} ${FILE_DIRECTORY})

	if (NOT REL_PATH STREQUAL "")
		set(REL_PATH ${REL_PATH}/)
		file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin/assets/${REL_PATH})
	endif ()

	set(OUT_DIR ${CMAKE_BINARY_DIR}/bin/assets/${REL_PATH})
	set(OUT_PATH ${OUT_DIR}${FILENAME})

	#add_custom_command(OUTPUT ${OUT_DIR}
	#		COMMAND ${CMAKE_COMMAND} -E make_directory ${OUT_DIR} COMMENT "Creating directory ${OUT_DIR}")

	if (EXISTS ${OUT_PATH})
		file(REMOVE ${OUT_PATH})
	endif ()

	add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/bin/assets/${FILENAME}
			COMMAND ${CMAKE_COMMAND} -E copy ${ASSET} ${OUT_PATH} DEPENDS ${ASSET}
			COMMENT "Copying ${ASSET} to ${OUT_PATH}")

	list(APPEND OUT_ASSETS ${CMAKE_BINARY_DIR}/bin/assets/${FILENAME})
	list(APPEND ADDITIONAL_CLEAN_FILES ${OUT_PATH})
endforeach ()

add_custom_target(assets ALL DEPENDS ${OUT_ASSETS})

