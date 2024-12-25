cmake_minimum_required(VERSION 3.0)
project(BaseServer)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y -g -Wall -O0 -Wno-unused-variable -pthread")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

include_directories(
	${PROJECT_SOURCE_DIR}/include/mysql
    ${PROJECT_SOURCE_DIR}/include/redis
	${PROJECT_SOURCE_DIR}/src/
	${PROJECT_SOURCE_DIR}/src/database
	${PROJECT_SOURCE_DIR}/src/moudle
	${PROJECT_SOURCE_DIR}/src/JSONforModernC++_3.11.3
	${PROJECT_SOURCE_DIR}/src/UTF8-CPP
	${PROJECT_SOURCE_DIR}/src/server
	${PROJECT_SOURCE_DIR}/net
)

aux_source_directory(${PROJECT_SOURCE_DIR}/src/server/database DB_LIST)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/server/moudle MODULE_LIST)
aux_source_directory(${PROJECT_SOURCE_DIR}/net NET_LIST)

add_subdirectory(src)