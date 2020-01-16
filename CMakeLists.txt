cmake_minimum_required(VERSION 2.8)

project(taskmaster CXX)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

set(CMAKE_CXX_STANDARD 17)

find_package(PkgConfig)

### yaml-cpp
find_package(yaml-cpp REQUIRED)
if (YAML_CPP_INCLUDE_DIR)
    message(STATUS "Yaml-cpp found. Libraries: ${YAML_CPP_LIBRARIES}. Headers: ${YAML_CPP_INCLUDE_DIR}.")
else()
    message(FATAL_ERROR "Yaml-cpp not found!")
endif()
include_directories(${YAML_CPP_INCLUDE_DIR})
# yaml-cpp end

### ZeroMQ
pkg_check_modules(PC_ZeroMQ QUIET zmq)
find_path(ZeroMQ_INCLUDE_DIR
          NAMES zmq.hpp
          PATHS ${PC_ZeroMQ_INCLUDE_DIRS}
         )
find_library(ZeroMQ_LIBRARY
             NAMES zmq
             PATHS ${PC_ZeroMQ_LIBRARY_DIRS}
            )
if (ZeroMQ_INCLUDE_DIR)
    message(STATUS "ZeroMQ found. Libraries: ${ZeroMQ_LIBRARY}. Headers: ${ZeroMQ_INCLUDE_DIR}.")
else()
    message(FATAL_ERROR "Yaml-cpp not found!")
endif()
include_directories(${ZeroMQ_INCLUDE_DIR})
# ZeroMq end

#include_directories(.)
add_executable(${PROJECT_NAME}
               src/main.cpp
               src/process.cpp
               src/task.cpp
               src/taskmaster.cpp
               src/communication.cpp
               config.yaml # for QtCreator
              )

target_link_libraries(${PROJECT_NAME}
                      pthread
                      ${YAML_CPP_LIBRARIES}
                      ${ZeroMQ_LIBRARY}
                     )
