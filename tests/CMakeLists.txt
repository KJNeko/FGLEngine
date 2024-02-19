

enable_testing()

file(GLOB_RECURSE FGL_TEST_SOURCES "*.cpp")

add_executable(FGLTests ${FGL_TEST_SOURCES})
target_link_libraries(FGLTests FGLEngine Catch2::Catch2WithMain)
target_compile_definitions(FGLTests PUBLIC GLM_FORCE_RADIANS GLM_FORCE_DEPTH_ZERO_TO_ONE GLM_FORCE_LEFT_HANDED)

include(CTest)
include(Catch)
catch_discover_tests(FGLTests)