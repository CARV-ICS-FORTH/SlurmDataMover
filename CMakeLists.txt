cmake_minimum_required(VERSION 3.17)

project(sdm)

set(CMAKE_CXX_STANDARD 11)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

#set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
# set(CMAKE_UNITY_BUILD FALSE)

find_package(Poco QUIET REQUIRED Net Util)

file(GLOB_RECURSE FMT_FILES src/*.h src/*.cpp PeerToPeer/*.h PeerToPeer/*.cpp)
add_custom_target(format ALL COMMAND clang-format -i ${FMT_FILES} WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

aux_source_directory(src SRC_FILES)
add_executable(sdm ${SRC_FILES})
target_link_libraries(sdm Poco::Util Poco::Net p2p)

add_subdirectory(PeerToPeer)
