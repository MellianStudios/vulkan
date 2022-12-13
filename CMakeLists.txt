cmake_minimum_required(VERSION 3.22.0)
project(MellianClient VERSION 0.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(Vulkan REQUIRED COMPONENTS glslc)
find_package(glm REQUIRED)
find_program(glslc_executable NAMES glslc HINTS Vulkan::glslc)

add_subdirectory(vendor/glfw)

link_libraries(glfw ${GLFW_LIBRARIES})
link_libraries(vulkan)

add_definitions(-DRELEASE -O2)
add_definitions(-DLINUX)

add_subdirectory(src)
