if (NOT DEFINED FGL_ENABLE_TESTS)
	set(FGL_ENABLE_TESTS 0)
endif ()


if (FGL_ENABLE_TESTS)
	message("-- FGL_ENABLE_TESTS: Enabled")
	enable_testing()

	file(GLOB_RECURSE FGL_TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/**.cpp")

	set(CMAKE_CXX_STANDARD_REQUIRED 23)

	add_executable(FGLTests ${FGL_TEST_SOURCES})
	target_link_libraries(FGLTests PUBLIC FGLEngine)
	target_link_libraries(FGLTests PRIVATE Catch2::Catch2WithMain)
	target_compile_definitions(FGLTests PUBLIC GLM_FORCE_RADIANS GLM_FORCE_DEPTH_ZERO_TO_ONE)
	target_compile_features(FGLTests PRIVATE cxx_std_23)

	include(CTest)
	include(Catch)
	catch_discover_tests(FGLTests)
else ()
	message("-- FGL_ENABLE_TESTS: Disabled")
endif ()