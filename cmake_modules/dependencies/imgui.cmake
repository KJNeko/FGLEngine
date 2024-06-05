
set(IMGUI_SOURCE ${CMAKE_SOURCE_DIR}/dependencies/imgui)

file(GLOB SOURCES ${IMGUI_SOURCE}/*.cpp)
list(APPEND SOURCES ${IMGUI_SOURCE}/backends/imgui_impl_glfw.cpp)
list(APPEND SOURCES ${IMGUI_SOURCE}/backends/imgui_impl_vulkan.cpp)
list(APPEND SOURCES ${IMGUI_SOURCE}/misc/cpp/imgui_stdlib.cpp)

add_library(ImGui OBJECT ${SOURCES})
target_link_libraries(ImGui PUBLIC glfw Vulkan::Vulkan)
target_include_directories(ImGui PUBLIC ${CMAKE_SOURCE_DIR}/dependencies)
target_include_directories(ImGui PUBLIC ${CMAKE_SOURCE_DIR}/dependencies/imgui)
