cmake_minimum_required(VERSION 3.10)
project(MinesweeperMultiplayer)

set(CMAKE_CXX_STANDARD 17)
find_package(Threads REQUIRED)

include_directories(include)

add_executable(server
    src/common/Board.cpp
    src/server/Server.cpp
    src/server/main_server.cpp
)
target_link_libraries(server PRIVATE Threads::Threads)

add_executable(client
    src/common/Board.cpp
    src/client/Client.cpp
    src/client/main_client.cpp
)
target_link_libraries(client PRIVATE Threads::Threads)
