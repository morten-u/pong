cmake_minimum_required(VERSION 3.25)
project(pong)

set(CMAKE_C_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(SDL3 REQUIRED)

add_executable(${PROJECT_NAME}
    pong.c
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    SDL3::SDL3
)
