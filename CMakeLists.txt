cmake_minimum_required(VERSION 2.8)

project(taskmaster CXX)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

set(CMAKE_CXX_STANDARD 17)

## RapidJSON
find_package(RapidJSON REQUIRED)
include_directories(${RAPIDJSON_INCLUDE_DIRS})

#include_directories(.)
add_executable(${PROJECT_NAME}
               src/main.cpp
               src/process.cpp
               src/task.cpp
              )

target_link_libraries(${PROJECT_NAME}
                      pthread
                     )
