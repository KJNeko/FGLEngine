

add_custom_target(FGLTests)

enable_testing()

file(GLOB_RECURSE FGL_TEST_SOURCES "*.cpp")

foreach (test_source ${FGL_TEST_SOURCES})
	get_filename_component(test_name ${test_source} NAME_WE)
	add_executable(${test_name} ${test_source})
	target_link_libraries(${test_name} FGLEngine)
	add_test(NAME ${test_name} COMMAND ${test_name})
	add_dependencies(FGLTests ${test_name})
endforeach (test_source)
