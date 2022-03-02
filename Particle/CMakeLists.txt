#cmake_minimum_required(VERSION 3.17)
#project(Particle)
#
#set(CMAKE_CXX_STANDARD 20)

file(GLOB CORE
        "src/*.cpp"
        "src/*.h"
        )

#link_directories(${PROJECT_SOURCE_DIR}/CMAKELib)
link_directories(${PROJECT_SOURCE_DIR}/CMAKE)

#option(SHATTER_PLATPORM_WINDOWS "Use Windows Platform" ON)
#option(SHATTER_BUILD_DLL "Build Shatter DLL" ON)

add_executable(Particle ${CORE})

target_link_libraries(Particle Shatter)

set(CMAKE_C_FLAGS "-static")
#set(CMAKE_LINK_LIBRARY_FLAG "-static")
set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
set(CMAKE_EXE_LINKER_FLAGS "-static")
