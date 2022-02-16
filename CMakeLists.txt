cmake_minimum_required(VERSION 3.17)
project(Particle)

set(CMAKE_CXX_STANDARD 20)

file(GLOB CORE
        "src/Engine/*/*.cpp"
        "src/Engine/*/*.h"
        "src/Platform/*/*.cpp"
        "src/Platform/*/*.h"
        )

include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui-master
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui-master/backends
)

file(GLOB IMGUI "external/imgui-master/*.cpp"
        "external/imgui-master/backends/imgui_impl_glfw.*"
        "external/imgui-master/backends/imgui_impl_vulkan.*"
        "external/imgui-master/*.h")

set(KTX_DIR external/ktx)
set(KTX_SOURCES
        ${KTX_DIR}/lib/texture.c
        ${KTX_DIR}/lib/hashlist.c
        ${KTX_DIR}/lib/checkheader.c
        ${KTX_DIR}/lib/swap.c
        ${KTX_DIR}/lib/memstream.c
        ${KTX_DIR}/lib/filestream.c
        )

add_library(ktx STATIC ${KTX_SOURCES})

add_executable(Particle
        ${CORE}
        ${IMGUI}
        main.cpp
        )

include_directories(src/)

include_directories(external/ktx/include/)

target_link_libraries(Particle ktx)

## STB
set(STB_INCLUDE_DIR external/stb-master/)
include_directories(${STB_INCLUDE_DIR})

## TINY_OBJ
set(TINY_OBJ_DIR external/tinyobjloader-master/)

include_directories(${TINY_OBJ_DIR})

#TINY_GLTF
set(TINY_GLTF_DIR external/tinygltf/)

include_directories(${TINY_GLTF_DIR})

#vulkangltf
include_directories(external/vulkangltf/)

## GLFW
set(GLFW_INCLUDE_DIRS external/glfw-3.3.4.bin.WIN64/include/GLFW/)
include_directories(${GLFW_INCLUDE_DIRS})

target_link_libraries(Particle glfw3)

## GLM
set(GLM_INCLUDE_DIRS external/glm/)
include_directories(${GLM_INCLUDE_DIRS})

## VulKan
find_package(Vulkan REQUIRED)
target_include_directories(Particle PUBLIC ${Vulkan_INCLUDE_DIRS})
target_link_libraries(Particle ${Vulkan_LIBRARIES})

set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
set(CMAKE_EXE_LINKER_FLAGS "-static")
