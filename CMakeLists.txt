cmake_minimum_required(VERSION 3.16)


set(PROJECT_NAME chip8)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(${PROJECT_NAME})

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# This assumes the SDL source is available in vendored/SDL
add_subdirectory(SDL3 EXCLUDE_FROM_ALL)
set(SOURCE_FILES src/main.c)
add_executable(chip8 ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)
