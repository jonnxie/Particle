cmake_minimum_required(VERSION 3.17)
project(Particle)

set(CMAKE_CXX_STANDARD 20)

file(GLOB CORE
        "Shatter/src/*.cpp"
        "Shatter/src/*.h"
        "Shatter/src/Engine/*/*.cpp"
        "Shatter/src/Engine/*/*.h"
        "Shatter/src/Platform/*/*.cpp"
        "Shatter/src/Platform/*/*.h"
        )

include_directories(
        Shatter/vendor/imgui-master
        Shatter/vendor/imgui-master/backends
)

file(GLOB IMGUI
        "Shatter/vendor/imgui-master/*.cpp"
        "Shatter/vendor/imgui-master/backends/imgui_impl_glfw.*"
        "Shatter/vendor/imgui-master/backends/imgui_impl_vulkan.*"
        "Shatter/vendor/imgui-master/*.h")

set(KTX_DIR Shatter/vendor/ktx)
set(KTX_SOURCES
        ${KTX_DIR}/lib/texture.c
        ${KTX_DIR}/lib/hashlist.c
        ${KTX_DIR}/lib/checkheader.c
        ${KTX_DIR}/lib/swap.c
        ${KTX_DIR}/lib/memstream.c
        ${KTX_DIR}/lib/filestream.c
        )

add_library(ktx STATIC ${KTX_SOURCES})

option(SHATTER_PLATPORM_WINDOWS "Use Windows Platform" ON)
option(SHATTER_BUILD_DLL "Build Shatter DLL" ON)

add_executable(Particle
        ${CORE}
        ${IMGUI}
        main.cpp
        )

include_directories(Shatter/src/)

include_directories(Shatter/vendor/ktx/include/)

target_link_libraries(Particle ktx)

## STB
set(STB_INCLUDE_DIR Shatter/vendor/stb-master/)
include_directories(${STB_INCLUDE_DIR})

## TINY_OBJ
set(TINY_OBJ_DIR Shatter/vendor/tinyobjloader-master/)

include_directories(${TINY_OBJ_DIR})

#TINY_GLTF
set(TINY_GLTF_DIR Shatter/vendor/tinygltf/)

include_directories(${TINY_GLTF_DIR})

##vulkangltf
#include_directories(external/vulkangltf/)

## GLFW
set(GLFW_INCLUDE_DIRS Shatter/vendor/glfw-3.3.4.bin.WIN64/include/)
include_directories(${GLFW_INCLUDE_DIRS})

target_link_libraries(Particle glfw3)

## GLM
set(GLM_INCLUDE_DIRS Shatter/vendor/glm/)
include_directories(${GLM_INCLUDE_DIRS})

## VulKan
find_package(Vulkan REQUIRED)
target_include_directories(Particle PUBLIC ${Vulkan_INCLUDE_DIRS})
target_link_libraries(Particle ${Vulkan_LIBRARIES})

set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
set(CMAKE_EXE_LINKER_FLAGS "-static")
