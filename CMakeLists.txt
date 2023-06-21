cmake_minimum_required(VERSION 3.16)

project(sdm)

set(CMAKE_CXX_STANDARD 11)

set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "Build SHARED libraries")
set(ENABLE_CRYPTO OFF CACHE INTERNAL "Enable Crypto")
set(ENABLE_XML OFF CACHE INTERNAL "Enable XML")
set(ENABLE_MONGODB OFF CACHE INTERNAL "Enable MongoDB")
set(ENABLE_CRYPTO OFF CACHE INTERNAL "Enable Crypto")
set(ENABLE_NETSSL OFF CACHE INTERNAL "Enable NetSSL")
set(ENABLE_JWT OFF CACHE INTERNAL "Enable JWT")
set(ENABLE_ZIP OFF CACHE INTERNAL "Enable ZIP")
set(ENABLE_PAGECOMPILER OFF CACHE INTERNAL "Enable PageCompiler")
set(ENABLE_PAGECOMPILER_FILE2PAGE OFF CACHE INTERNAL "Enable File2Page")
set(ENABLE_ACTIVERECORD OFF CACHE INTERNAL "Enable ActiveRecord")
set(ENABLE_ACTIVERECORD_COMPILER OFF CACHE INTERNAL "Enable ActiveRecord Compiler")
set(ENABLE_DATA OFF CACHE INTERNAL "Enable Data")
set(ENABLE_DATA_MYSQL OFF CACHE INTERNAL "Enable MariaDB")
set(ENABLE_DATA_SQLITE OFF CACHE INTERNAL "Enable Data SQlite")
set(ENABLE_DATA_ODBC  OFF CACHE INTERNAL "Enable Data ODBC")
set(ENABLE_ENCODINGS OFF CACHE INTERNAL "Enable Encodings")
include(FetchContent)

FetchContent_Declare(Poco
  GIT_REPOSITORY    https://github.com/pocoproject/poco.git
  GIT_TAG           poco-1.11.6-release
  GIT_SHALLOW       TRUE
)

FetchContent_MakeAvailable(Poco)

find_program(FORMATER clang-format)

if(FORMATER)
	file(GLOB_RECURSE FMT_FILES src/*.h src/*.cpp PeerToPeer/*.h PeerToPeer/*.cpp)
	add_custom_target(format ALL COMMAND clang-format -i ${FMT_FILES} WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
else()
	message("-- clang-format not found - Formatting skipped")
endif()

aux_source_directory(src SRC_FILES)
add_executable(sdm ${SRC_FILES})
target_link_libraries(sdm Poco::Util Poco::Net Poco::Redis)
