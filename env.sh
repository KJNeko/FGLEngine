

alias setup_vk="source /home/kj16609/VulkanSDK/Releases/latest/setup-env.sh"
setup_vk
alias qrenderdoc="setup_vk && /usr/bin/qrenderdoc"

export TRACY_BUILD_DIR=/home/kj16609/Desktop/Projects/cxx/Mecha/dependencies/tracy/profiler
cmake $TRACY_BUILD_DIR -B $TRACY_BUILD_DIR/build
cmake --build $TRACY_BUILD_DIR/build -j32

alias tracy="~/Desktop/Projects/cxx/Mecha/dependencies/tracy/profiler/build/tracy-profiler"



