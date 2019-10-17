cmake_minimum_required(VERSION 2.8)

include_directories(.)

project(Taskmaster)
add_executable(${PROJECT_NAME}
               main.cpp
               process.cpp
              )
