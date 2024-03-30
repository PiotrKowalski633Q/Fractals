cmake_minimum_required(VERSION 3.25)
project(Fractals)

set(CMAKE_CXX_STANDARD 23)

set(dir ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${dir}/Build)

set(SFML_STATIC_LIBRARIES TRUE)
set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")
set(SFML_DIR "Dependencies/SFML-2.6.0/lib/cmake/SFML")

file(COPY Resources DESTINATION ${dir}/Build)
file(COPY Dependencies/SFML-2.6.0/bin/openal32.dll DESTINATION ${dir}/Build)

find_package(SFML 2.6.0 REQUIRED COMPONENTS audio graphics window system)

include_directories(Dependencies/SFML-2.6.0/include)
include_directories(Dependencies/OpenCL/include)


add_executable(Fractals
        Code/Sources/main.cpp
        Code/Headers/Game.h
        Code/Sources/Game.cpp
        Code/Headers/FractalCanvas.h
        Code/Sources/FractalCanvas.cpp
        Code/Headers/OpenCLFunctions.h
        Code/Sources/OpenCLFunctions.cpp)

target_link_libraries(Fractals sfml-audio sfml-graphics sfml-window sfml-system)
target_link_libraries(Fractals ${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/OpenCL/lib/OpenCL.lib)