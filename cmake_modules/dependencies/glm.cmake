if (WIN32)
	add_subdirectory(${CMAKE_SOURCE_DIR}/dependencies/glm)
else ()
	find_package(glm REQUIRED)
endif ()