
file(GLOB_RECURSE CPP_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/**.cpp")

add_library(FGLLoader STATIC ${CPP_SOURCES})

target_include_directories(FGLLoader PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
set_target_properties(FGLLoader PROPERTIES COMPILE_FLAGS ${FGL_CHILD_FLAGS})


