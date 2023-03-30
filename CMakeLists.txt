cmake_minimum_required(VERSION 3.13)

project(crsf C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

target_include_directories(crsf PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include )
target_include_directories(crsf PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src )

target_link_libraries(crsf)
