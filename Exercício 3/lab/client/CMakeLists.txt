set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -pthread -std=c++11")

include_directories(include)
file(GLOB SOURCES "src/*.cpp")

add_executable(client ${SOURCES})
target_link_libraries(client socketutils)
