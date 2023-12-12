# Main CMakeLists file

# set minimum version requirement
cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

# create top level project named WindowMTSystem
project(WindowMTSystem VERSION 1.0.0.0)

# include the subdirectories
# these will be the projects akin to visual studio projects
add_subdirectory(projects/Example1)
add_subdirectory(projects/v1.0)




