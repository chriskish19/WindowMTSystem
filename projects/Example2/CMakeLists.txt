# Example2 project Cmake script

# create the project
project(Example2 VERSION 1.0.0.0)

# Set the variable CMAKE_CXX_STANDARD to c++20
# and the variable CMAKE_CXX_STANDARD_REQUIRED to True
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)

# Add the source files here
set(SOURCE_FILES src/main.cpp
                 src/iWindow.hpp
                 src/resource.h
                 src/Example2.rc
                 src/WindowDimensions.cpp
                 src/WindowDimensions.hpp
                 src/WindowResources.cpp
                 src/WindowResources.hpp
                 src/logger.hpp
                 src/logger.cpp)

# Create an executable
add_executable(Example2 ${SOURCE_FILES})

# Set the Windows subsystem to "windows"
set_target_properties(Example2 PROPERTIES WIN32_EXECUTABLE true)

# Specify that the resource file uses the RC language
set_source_files_properties(src/Example2.rc PROPERTIES LANGUAGE RC) 

# Define UNICODE macro
add_compile_definitions(UNICODE _UNICODE)