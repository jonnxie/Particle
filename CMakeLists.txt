cmake_minimum_required(VERSION 3.17)
project(main)

set(CMAKE_CXX_STANDARD 20)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

#set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/CMAKELib)
#set(EXECUTABLE_OUTPUT_PATH  ${PROJECT_SOURCE_DIR}/CMAKEBin)
set(LIBRARY_OUTPUT_PATH     ${PROJECT_SOURCE_DIR}/CMAKE)
set(EXECUTABLE_OUTPUT_PATH  ${PROJECT_SOURCE_DIR}/CMAKE)

add_subdirectory(Shatter)
add_subdirectory(Particle)

set(CMAKE_C_FLAGS "-static")
#set(CMAKE_LINK_LIBRARY_FLAG "-static")
set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
set(CMAKE_EXE_LINKER_FLAGS "-static")