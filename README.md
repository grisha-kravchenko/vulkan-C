# Vulkan renderer
Simple vulkan rendering project written in pure C.

## Current capabilities
Current code only creates semi-animated window filled with
color dependent on time. The code is currently only
works with linux wayland, which is going to be changed
soon.

## Build the project
*Warning: build script is written for linux only.*

Make sure gcc is installed.

First time you need to build the build script using gcc:
`gcc build.c -o build`
But afterwards building the project is just `./build`.
The build script will recompile itself when modified.
You can use `./build -r` to automatically run the compiled
binary.

## Additional notice
The project uses [Vulkan Memory Allocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator),
and destributes its copy within a git repository.

The project uses [Lua Language](https://github.com/lua/lua)

