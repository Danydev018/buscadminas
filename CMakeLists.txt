cmake_minimum_required(VERSION 3.10)
project(MinesweeperMultiplayer)

set(CMAKE_CXX_STANDARD 17)
find_package(Threads REQUIRED)

include_directories(include)


add_executable(buscadminas_unificado
    src/common/Board.cpp
    src/client/Client.cpp
    src/server/Server.cpp
    src/main.cpp
    src/client/main_client.cpp
    src/server/main_server.cpp
    src/common/ConsoleUtils.cpp
    src/common/NetworkUtils.cpp
    src/common/ScoreSystem.cpp
)
target_link_libraries(buscadminas_unificado PRIVATE Threads::Threads)
