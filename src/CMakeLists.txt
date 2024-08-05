
add_subdirectory(vma)
add_subdirectory(engine)
add_subdirectory(objectloaders)
add_subdirectory(editor)
add_subdirectory(tests)
add_subdirectory(shaders)
add_subdirectory(assets)

add_dependencies(TitorEditor shaders)
add_dependencies(TitorEditor assets)
