cmake_minimum_required(VERSION 3.10)  # Specify CMake version
project(serial-reader VERSION 1.0 LANGUAGES C)  # Project name and version

set(CMAKE_C_STANDARD 99)

# Add executable
add_executable(main
    ${PROJECT_SOURCE_DIR}/src/main.c
    ${PROJECT_SOURCE_DIR}/src/parser.c
    ${PROJECT_SOURCE_DIR}/src/serial.c
)

target_compile_features(main PRIVATE c_std_99)

set_target_properties(main PROPERTIES
    OUTPUT_NAME "serial-reader"
)

# Include directories
target_include_directories(main PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)