set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH  ${CMAKE_BINARY_DIR}/lib)
add_compile_options(-fpermissive)

aux_source_directory(client CLIENT_SRC)
aux_source_directory(server SERVER_SRC)
add_library(p2p SHARED ${CLIENT_SRC} ${SERVER_SRC})
target_include_directories(p2p PUBLIC client server)


#add_executable(get P2Pmain.cpp)

#target_link_libraries(get client)

