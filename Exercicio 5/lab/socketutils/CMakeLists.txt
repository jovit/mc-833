
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -std=c++11")

file(GLOB SOURCES "src/*.cpp")

add_library(socketutils SHARED ${SOURCES})
target_include_directories(socketutils PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
