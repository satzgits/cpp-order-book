cmake_minimum_required(VERSION 3.16)
project(cpp-order-book VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(order_book
    src/main.cpp
    src/order.hpp
    src/order_book.hpp
    src/matching.hpp
)

target_include_directories(order_book PRIVATE src)

if(MSVC)
    target_compile_options(order_book PRIVATE /W4 /O2)
else()
    target_compile_options(order_book PRIVATE -Wall -Wextra -O3 -march=native)
endif()

enable_testing()
add_executable(test_order_book tests/test_order_book.cpp src/order_book.hpp src/matching.hpp)
target_include_directories(test_order_book PRIVATE src)
