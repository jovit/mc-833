set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -std=c++11")

include_directories(include)
file(GLOB SOURCES "src/*.cpp")

add_executable(server ${SOURCES})
target_link_libraries(server socketutils)
