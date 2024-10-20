
add_subdirectory(vma)
add_subdirectory(engine)
add_subdirectory(objectloaders)
add_subdirectory(editor)
add_subdirectory(tests)

message("-- Creating SYMLINK ${CMAKE_BINARY_DIR}/shaders -> ${CMAKE_CURRENT_SOURCE_DIR}/shaders")
file(CREATE_LINK ${CMAKE_CURRENT_SOURCE_DIR}/shaders ${CMAKE_BINARY_DIR}/bin/shaders SYMBOLIC)

message("-- Creating SYMLINK ${CMAKE_BINARY_DIR}/assets -> ${CMAKE_CURRENT_SOURCE_DIR}/assets")
file(CREATE_LINK ${CMAKE_CURRENT_SOURCE_DIR}/assets ${CMAKE_BINARY_DIR}/bin/assets SYMBOLIC)