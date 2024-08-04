

set(CMAKE_CXX_STANDARD 23)

target_compile_definitions(FGLEngine PUBLIC VULKAN_HPP_FLAGS_MASK_TYPE_AS_PUBLIC)

file(GLOB_RECURSE SOURCE_FILES
		"${CMAKE_CURRENT_SOURCE_DIR}/**.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/**.hpp"
)

add_executable(TitorEditor ${SOURCE_FILES})
target_link_libraries(TitorEditor PRIVATE FGLEngine)
target_include_directories(TitorEditor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_definitions(TitorEditor PUBLIC TITOR_EDITOR)
target_compile_features(TitorEditor PRIVATE cxx_std_23)
