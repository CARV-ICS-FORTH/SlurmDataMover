cmake_minimum_required(VERSION 3.0)
include(CMakePrintHelpers)

project(GETFILE CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH  ${CMAKE_BINARY_DIR}/lib)
add_compile_options(-fpermissive)
message(${CMAKE_BINARY_DIR})

add_library(client SHARED PeerClient.cpp  getData.cpp)

#add_executable(get P2Pmain.cpp)   
             
#target_link_libraries(get client)

