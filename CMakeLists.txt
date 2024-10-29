CMAKE_minimum_required(VERSION 3.1...3.29)

project(
    ConsoleTetris
    VERSION 1.0
    LANGUAGES CXX
)

file(GLOB_RECURSE PROJECT_SRC CONFIGURE_DEPENDS "src/*.cpp")
list(APPEND PROJECT_SRC "main.cpp")
set(PROJECT_INC "include")
set(PROJECT_LIB "")
set(PROJECT_CFLAGS "-std=c++20")

set(CURSES_NEED_WIDE TRUE)
find_package(Curses REQUIRED)
list(APPEND PROJECT_INC ${CURSES_INCLUDE_DIRS})
list(APPEND PROJECT_LIB ${CURSES_LIBRARIES})
list(APPEND PROJECT_CFLAGS ${CURSES_CFLAGS})

add_executable(${PROJECT_NAME} ${PROJECT_SRC})

target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INC})
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_LIB})
target_compile_options(${PROJECT_NAME} PRIVATE ${PROJECT_CFLAGS})
