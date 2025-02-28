

set(GLFW_BUILD_X11 ON)
set(GLFW_BUILD_WAYLAND OFF)

#if (WIN32)
	if (DEFINED ENV{GLFW_PATH})
		message("-- GLFW_PATH defined as: $ENV{GLFW_PATH}.")
		list(APPEND CMAKE_PREFIX_PATH $ENV{GLFW_PATH})
		find_package(glfw3 REQUIRED)
	else ()
		message("-- GLFW_PATH not defined. Using submodule instead")
		add_subdirectory(${CMAKE_SOURCE_DIR}/dependencies/glfw3)
	endif ()
#else ()
#	find_package(glfw3 REQUIRED)
#endif ()

set(GLFW_BUILD_X11 ON)
set(GLFW_BUILD_WAYLAND OFF)
