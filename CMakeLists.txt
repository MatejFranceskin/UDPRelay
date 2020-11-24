cmake_minimum_required(VERSION 3.16)

project(UDPRelay)

find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD 17)

include_directories(
)

add_executable(${PROJECT_NAME}
    main.cpp
    UDPRelay.cpp
)

target_link_libraries(${PROJECT_NAME}
    ${CMAKE_THREAD_LIBS_INIT}
)
