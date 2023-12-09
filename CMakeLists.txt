cmake_minimum_required(VERSION 3.25)
project(MROS CXX)

set(SOCKET_SOURCES
        src/socket/bson_socket/bson_socket.cpp
        src/socket/bson_socket/client_bson_socket.cpp
        src/socket/bson_socket/connection_bson_socket.cpp
        src/socket/bson_socket/server_bson_socket.cpp
        src/socket/json_rpc_socket/client_json_rpc_socket.cpp
        src/socket/json_rpc_socket/connection_json_rpc_socket.cpp
        src/socket/json_rpc_socket/json_rpc_socket.cpp
        src/socket/json_rpc_socket/server_json_rpc_socket.cpp
        src/socket/message_socket/message_socket.cpp
        src/socket/rpc_socket/rpc_socket.cpp
        src/socket/client_socket.cpp
        src/socket/connection_socket.cpp
        src/socket/server_socket.cpp
        src/socket/socket.cpp
)

add_library(mroslib SHARED
        src/logging/logging.cpp
        src/mros/mros.cpp
        src/mros/node.cpp
        src/mros/publisher.cpp
        src/mros/subscriber.cpp
        ${SOCKET_SOURCES}
)
target_include_directories(mroslib PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include/mros>
)

add_executable(mroscore
        src/logging/logging.cpp
        src/mediator/mediator.cpp
        src/mros/mros.cpp
        ${SOCKET_SOURCES}
)
target_include_directories(mroscore
        PRIVATE
        include
)

add_executable(mroskill
        src/command_line/mroskill.cpp
        src/logging/logging.cpp
        src/mros/mros.cpp
        src/mros/node.cpp
        src/mros/publisher.cpp
        src/mros/subscriber.cpp
        ${SOCKET_SOURCES}
)
target_include_directories(mroskill
        PRIVATE
        include
)

add_executable(mrosnode
        src/command_line/mrosnode.cpp
        src/logging/logging.cpp
        src/mros/mros.cpp
        src/mros/node.cpp
        src/mros/publisher.cpp
        src/mros/subscriber.cpp
        ${SOCKET_SOURCES}
)
target_include_directories(mrosnode
        PRIVATE
        include
)

add_executable(mrostopic
        src/command_line/mrostopic.cpp
        src/logging/logging.cpp
        src/mros/mros.cpp
        src/mros/node.cpp
        src/mros/publisher.cpp
        src/mros/subscriber.cpp
        ${SOCKET_SOURCES}
)
target_include_directories(mrostopic
        PRIVATE
        include
)
