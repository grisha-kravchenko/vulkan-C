# Vulkan renderer
Simple vulkan rendering project written in pure C.

## Build the project
*Warning: build script is written for linux only.*

Make sure gcc is installed.

First time you need to build the build script using gcc:
`gcc build.c -o build`
But afterwards building the project is just `./build`.
The build script will recompile itself when modified.
You can use `./build -r` to automatically run the compiled
binary.
