# Example1 project Cmake script

# create the project
project(Example1 VERSION 1.0.0.0)

# Set the variable CMAKE_CXX_STANDARD to c++20
# and the variable CMAKE_CXX_STANDARD_REQUIRED to True
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)

# Add the source files here
set(SOURCE_FILES src/main.cpp
                 src/iWindow.hpp
                 src/resource.h
                 src/Example1.rc)

# Create an executable
add_executable(Example1 ${SOURCE_FILES})

# Set the Windows subsystem to "windows"
set_target_properties(Example1 PROPERTIES WIN32_EXECUTABLE true)

# Specify that the resource file uses the RC language
set_source_files_properties(src/Example1.rc PROPERTIES LANGUAGE RC)

# Define UNICODE macro
add_compile_definitions(UNICODE _UNICODE)