

set(CMAKE_CXX_STANDARD 23)

target_compile_definitions(FGLEngine PUBLIC VULKAN_HPP_FLAGS_MASK_TYPE_AS_PUBLIC)

file(GLOB_RECURSE SOURCE_FILES
		"${CMAKE_CURRENT_SOURCE_DIR}/**.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/**.hpp"
)

AddFGLExecutable(TitorEditor ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(TitorEditor PRIVATE FGLEngine)
target_compile_definitions(TitorEditor PUBLIC TITOR_EDITOR)
target_compile_features(TitorEditor PRIVATE cxx_std_23)
target_link_libraries(TitorEditor PRIVATE glfw ImGui)
