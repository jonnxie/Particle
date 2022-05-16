#cmake_minimum_required(VERSION 3.17)
#project(Shatter)
#
#set(CMAKE_CXX_STANDARD 20)

file(GLOB CORE
        "src/*.cpp"
        "src/*.h"
        "src/Engine/*/*.cpp"
        "src/Engine/*/*.h"
        "src/Platform/*/*.cpp"
        "src/Platform/*/*.h"
        )
file(GLOB IMGUI
        "vendor/imgui-docking/*.cpp"
        "vendor/imgui-docking/backends/imgui_impl_glfw.*"
        "vendor/imgui-docking/backends/imgui_impl_vulkan.*"
        "vendor/imgui-docking/*.h")

set(KTX_DIR vendor/ktx)
set(KTX_SOURCES
        ${KTX_DIR}/lib/texture.c
        ${KTX_DIR}/lib/hashlist.c
        ${KTX_DIR}/lib/checkheader.c
        ${KTX_DIR}/lib/swap.c
        ${KTX_DIR}/lib/memstream.c
        ${KTX_DIR}/lib/filestream.c
        ${KTX_DIR}/include/ktx.h
        ${KTX_DIR}/include/ktxvulkan.h
        )

add_library(ktx STATIC ${KTX_SOURCES})

set(REFLECT_DIR vendor/spirv_reflect)
set(REFLECT_SOURCES
        ${REFLECT_DIR}/spirv_reflect.h
        ${REFLECT_DIR}/spirv_reflect.c)
add_library(reflect STATIC ${REFLECT_SOURCES})

option(USE_OFFSCREEN "Build Offscreen Rendering" OFF)
option(USE_SHADOW "Build Shadow Map DLL" OFF)
option(USE_SIMD "USE Simd In Math Calculation" OFF)
option(USE_GPU_CAPTURE "USE GPU Capture" ON)

add_compile_definitions(SHATTER_PLATFORM_WINDOWS)
add_compile_definitions(SHATTER_GRAPHICS_VULKAN)
add_compile_definitions(SHATTER_BUILD_DLL)

if(USE_OFFSCREEN)
    add_compile_definitions(SHATTER_OFFSCREEN)
endif()

if(USE_SHADOW)
    add_compile_definitions(SHATTER_SHADOW)
endif()

if(USE_SIMD)
    add_compile_definitions(SHATTER_SIMD)
endif()

if(USE_GPU_CAPTURE)
    add_compile_definitions(SHATTER_GPU_CAPTURE)
endif()

add_library(Shatter SHARED ${CORE} ${IMGUI})

## VulKan
find_package(Vulkan REQUIRED)

target_include_directories(Shatter PUBLIC
        vendor/imgui-docking
        vendor/imgui-docking/backends
        src/
        vendor/ktx/include/
        vendor/stb-master/
        vendor/tinyobjloader-master/
        vendor/tinygltf/
        vendor/glfw-3.3.4.bin.WIN64/include/
        vendor/glm/
        vendor/Entt/
        vendor/json/
        ${Vulkan_INCLUDE_DIRS}
        ${REFLECT_DIR})

target_link_libraries(Shatter
        ktx
        glfw3
        ${Vulkan_LIBRARIES}
        reflect
        )

set(CMAKE_C_FLAGS "-static")
#set(CMAKE_LINK_LIBRARY_FLAG "-static")
set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
set(CMAKE_EXE_LINKER_FLAGS "-static")
