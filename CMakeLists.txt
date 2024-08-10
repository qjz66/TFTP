cmake_minimum_required(VERSION 3.23)
project(Socket_Programming)

set(CMAKE_CXX_STANDARD 14)
# ָ��libĿ¼
link_directories(${PROJECT_SOURCE_DIR}/lib)

add_executable(Socket_Programming TFTP.cpp
        1.cpp
        3.cpp)
# �������ӵ���Ŀ��
target_link_libraries(${PROJECT_NAME} libws2_32.a)

add_executable(1 1.cpp)
add_executable(3 3.cpp)