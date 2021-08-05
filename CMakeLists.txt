cmake_minimum_required(VERSION 3.1)

project(backtracexx)

add_library(${PROJECT_NAME} SHARED "backtracexx.cpp")
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(${PROJECT_NAME}_example "example.cpp")
set_target_properties(${PROJECT_NAME} PROPERTIES ENABLE_EXPORTS 1)
target_link_libraries(${PROJECT_NAME}_example ${PROJECT_NAME})

