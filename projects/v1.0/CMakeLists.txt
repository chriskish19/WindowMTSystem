# v1.0 project Cmake script

# create the project
project(v1.0 VERSION 1.0.0.0)

# Set the variable CMAKE_CXX_STANDARD to c++20
# and the variable CMAKE_CXX_STANDARD_REQUIRED to True
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)

# Add the source files here
set(SOURCE_FILES src/main.cpp
                 src/iWindow.hpp)

# Create an executable
add_executable(v1.0 ${SOURCE_FILES})

# Set the Windows subsystem to "windows"
set_target_properties(v1.0 PROPERTIES WIN32_EXECUTABLE true)

# Define UNICODE macro
add_compile_definitions(UNICODE _UNICODE)
