

file(GLOB CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(FGLImGui STATIC ${CPP_SOURCES})
target_link_libraries(FGLImGui PUBLIC glfw Vulkan::Vulkan)
