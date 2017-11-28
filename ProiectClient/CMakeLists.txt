cmake_minimum_required(VERSION 3.6.1)
project(ProiectClient)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES main.c)
add_executable(ProiectClient ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} "-lws2_32")
