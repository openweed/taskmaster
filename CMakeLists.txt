cmake_minimum_required(VERSION 2.8)

project(taskmaster CXX)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

set(CMAKE_CXX_STANDARD 17)

### RapidJSON
find_package(RapidJSON REQUIRED)
include_directories(${RAPIDJSON_INCLUDE_DIRS})
# RapidJSON end

### yaml-cpp
find_package(yaml-cpp REQUIRED)
if (YAML_CPP_INCLUDE_DIR)
    message(STATUS "Yaml-cpp found. Libraries: ${YAML_CPP_LIBRARIES}. Headers: ${YAML_CPP_INCLUDE_DIR}.")
else()
    message(STATUS "Yaml-cpp not found!")
endif()
include_directories(${YAML_CPP_INCLUDE_DIR})
# yaml-cpp end

#include_directories(.)
add_executable(${PROJECT_NAME}
               src/main.cpp
               src/process.cpp
               src/task.cpp
               src/taskmaster.cpp
              )

target_link_libraries(${PROJECT_NAME}
                      pthread
                      ${YAML_CPP_LIBRARIES}
                     )
