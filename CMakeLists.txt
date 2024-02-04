cmake_minimum_required(VERSION 3.25)
project(MROS CXX)
set(CMAKE_CXX_STANDARD 20)

find_package(nlohmann_json REQUIRED CONFIG)
find_package(log4cxx CONFIG REQUIRED)
#find_package(EXPAT REQUIRED)
find_package(Threads REQUIRED)
find_package(GTest REQUIRED)

# ---------------------------- Socket Library ----------------------------
add_library(mros_socket STATIC
        src/logging/logging.cpp
        src/socket/bson_rpc_socket/bson_rpc_socket.cpp
        src/socket/bson_rpc_socket/client_bson_rpc_socket.cpp
        src/socket/bson_rpc_socket/connection_bson_rpc_socket.cpp
        src/socket/bson_socket/bson_socket.cpp
        src/socket/bson_socket/client_bson_socket.cpp
        src/socket/bson_socket/connection_bson_socket.cpp
        src/socket/client_socket.cpp
        src/socket/connection_socket.cpp
        src/socket/server_socket.cpp
        src/socket/socket.cpp
)
target_include_directories(mros_socket PUBLIC include)
target_link_libraries(mros_socket PUBLIC log4cxx)

# ---------------------------- MROS Library ----------------------------
#add_library(mroslib
#        SHARED
#        src/logging/logging.cpp
#        src/mros/mros.cpp
#        src/mros/node.cpp
#        src/mros/publisher.cpp
#        src/mros/subscriber.cpp
#        src/mros/utils/utils.cpp
#        ${SOCKET_SOURCES}
#)
#target_include_directories(mroslib
#        PUBLIC
#        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
#        $<INSTALL_INTERFACE:include/mros>
#)
#target_link_libraries(mroslib
#        PRIVATE
#        nlohmann_json::nlohmann_json
#        log4cxx
#        EXPAT::EXPAT
#)
#
# ---------------------------- Command Line Executables ----------------------------
# add_executable(mroscore
#         src/logging/logging.cpp
#         src/mediator/mediator.cpp
#         src/mros/mros.cpp
#         ${SOCKET_SOURCES}
# )
# target_include_directories(mroscore
#         PRIVATE
#         include
# )
# target_link_libraries(mroscore
#         PRIVATE
#         nlohmann_json::nlohmann_json
#         log4cxx
#         EXPAT::EXPAT
# )
#
# add_executable(mroskill
#         src/command_line/mroskill.cpp
#         src/logging/logging.cpp
#         src/mros/mros.cpp
#         src/mros/node.cpp
#         src/mros/publisher.cpp
#         src/mros/subscriber.cpp
#         ${SOCKET_SOURCES}
# )
# target_include_directories(mroskill
#         PRIVATE
#         include
# )
# target_link_libraries(mroskill
#         PRIVATE
#         nlohmann_json::nlohmann_json
#         log4cxx EXPAT::EXPAT
# )
#
# add_executable(mrosnode
#         src/command_line/mrosnode.cpp
#         src/logging/logging.cpp
#         src/mros/mros.cpp
#         src/mros/node.cpp
#         src/mros/publisher.cpp
#         src/mros/subscriber.cpp
#         ${SOCKET_SOURCES}
# )
# target_include_directories(mrosnode
#         PRIVATE
#         include
# )
# target_link_libraries(mrosnode
#         PRIVATE
#         nlohmann_json::nlohmann_json
#         log4cxx
#         EXPAT::EXPAT
# )
#
# add_executable(mrostopic
#         src/command_line/mrostopic.cpp
#         src/logging/logging.cpp
#         src/mros/mros.cpp
#         src/mros/node.cpp
#         src/mros/publisher.cpp
#         src/mros/subscriber.cpp
#         ${SOCKET_SOURCES}
# )
# target_include_directories(mrostopic
#         PRIVATE
#         include
# )
# target_link_libraries(mrostopic
#         PRIVATE
#         nlohmann_json::nlohmann_json
#         log4cxx
#         EXPAT::EXPAT
# )
#
# ---------------------------- Automated Unit Tests ----------------------------
enable_testing()
add_executable(test_bson_socket test/socket/bson_socket/test_bson_socket.cpp)
target_link_libraries(test_bson_socket GTest::gtest_main mros_socket)
gtest_discover_tests(test_bson_socket)

add_executable(test_bson_rpc_socket test/socket/bson_rpc_socket/test_bson_rpc_socket.cpp)
target_link_libraries(test_bson_rpc_socket PUBLIC GTest::gtest_main mros_socket)
gtest_discover_tests(test_bson_rpc_socket)

# ---------------------------- Manual Unit Tests ----------------------------
#add_executable(test_json_rpc_client
#        ${SOCKET_SOURCES}
#        test_manual/socket/json_rpc_socket/test_json_rpc_client.cpp
#        src/logging/logging.cpp
#)
#target_include_directories(test_json_rpc_client
#        PRIVATE
#        include
#)
#target_link_libraries(test_json_rpc_client
#        PRIVATE
#        ${CMAKE_THREAD_LIBS_INIT}
#        nlohmann_json::nlohmann_json
#        log4cxx
#        EXPAT::EXPAT
#)
#
#add_executable(test_json_rpc_server
#        ${SOCKET_SOURCES}
#        test_manual/socket/json_rpc_socket/test_json_rpc_server.cpp
#        src/logging/logging.cpp
#)
#target_include_directories(test_json_rpc_server
#        PRIVATE
#        include
#)
#target_link_libraries(test_json_rpc_server
#        PRIVATE
#        ${CMAKE_THREAD_LIBS_INIT}
#        nlohmann_json::nlohmann_json
#        log4cxx
#        EXPAT::EXPAT
#)
#
#add_executable(mros_node_test
#        src/logging/logging.cpp
#        src/mros/node.cpp
#        src/mros/mros.cpp
#        test/mros/test_node.cpp
#        ${SOCKET_SOURCES}
#)
#target_include_directories(mros_node_test
#        PRIVATE
#        include
#)
#target_link_libraries(mros_node_test
#        PRIVATE
#        nlohmann_json::nlohmann_json
#        log4cxx
#        EXPAT::EXPAT
#)
#
#add_executable(mros_mediator_test
#        src/logging/logging.cpp
#        src/mros/mros.cpp
#        src/mediator/mediator.cpp
#        test/mediator/test_mediator.cpp
#        src/mros/utils/utils.cpp
#        ${SOCKET_SOURCES}
#)
#target_include_directories(mros_mediator_test
#        PRIVATE
#        include
#)
#target_link_libraries(mros_mediator_test
#        PRIVATE
#        nlohmann_json::nlohmann_json
#        log4cxx
#        EXPAT::EXPAT
#)
#
#add_executable(test_logger
#        test_manual/logger/test_logger.cpp
#        src/logging/logging.cpp
#)
#target_include_directories(test_logger
#        PRIVATE
#        include
#)
#target_link_libraries(test_logger
#        PRIVATE
#        log4cxx
#        EXPAT::EXPAT
#)
#
#
#add_executable(mros_pub_test
#        src/logging/logging.cpp
#        src/mros/node.cpp
#        src/mros/mros.cpp
#        src/mros/publisher.cpp
#        test/mros/test_publisher.cpp
#        ${SOCKET_SOURCES}
#)
#target_include_directories(mros_pub_test
#        PRIVATE
#        include
#)
#target_link_libraries(mros_pub_test
#        PRIVATE
#        nlohmann_json::nlohmann_json
#        log4cxx
#        EXPAT::EXPAT
#)
#
#add_executable(mros_sub_test
#        src/logging/logging.cpp
#        src/mros/node.cpp
#        src/mros/mros.cpp
#        src/mros/subscriber.cpp
#        test/mros/test_subscriber.cpp
#        ${SOCKET_SOURCES}
#)
#target_include_directories(mros_sub_test
#        PRIVATE
#        include
#)
#target_link_libraries(mros_sub_test
#        PRIVATE
#        nlohmann_json::nlohmann_json
#        log4cxx
#        EXPAT::EXPAT
#)
