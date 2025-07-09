cmake_minimum_required(VERSION 3.10)
project(BuscaminasMultijugador)

set(/usr/include/boost)

# Directorios de cabeceras
include_directories(include)

# Archivos fuente
file(GLOB SRC_FILES src/*.cpp)

# Opciones para Termux/Android
add_definitions(-DTERMUX)

# Dear ImGui (requiere que el usuario lo tenga descargado o lo incluya manualmente)
# Puedes cambiar la ruta a donde tengas Dear ImGui
# set(IMGUI_DIR ../imgui)
# include_directories(${IMGUI_DIR})
# file(GLOB IMGUI_SRC ${IMGUI_DIR}/*.cpp)
# list(APPEND SRC_FILES ${IMGUI_SRC})

# Boost (para sockets y utilidades)
find_package(Boost REQUIRED COMPONENTS system)
include_directories(${Boost_INCLUDE_DIRS})

# Cliente
add_executable(buscaminas_client client/main_client.cpp ${SRC_FILES})
target_link_libraries(buscaminas_client ${Boost_LIBRARIES})

# Servidor
add_executable(buscaminas_server server/main_server.cpp ${SRC_FILES})
target_link_libraries(buscaminas_server ${Boost_LIBRARIES})
