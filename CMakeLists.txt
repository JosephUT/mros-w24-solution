cmake_minimum_required(VERSION 3.25)
project(MROS CXX)
set(CMAKE_CXX_STANDARD 20)

find_package(nlohmann_json REQUIRED CONFIG)
find_package(log4cxx CONFIG REQUIRED)
find_package(EXPAT REQUIRED)
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
target_link_libraries(mros_socket PUBLIC log4cxx EXPAT::EXPAT)

# ---------------------------- MROS Library ----------------------------

# ---------------------------- Command Line Executables ----------------------------
add_executable(mroscore
        src/command_line/mroscore.cpp
        src/mediator/mediator.cpp
        src/mros/mros.cpp
        src/mros/utils/utils.cpp
)
target_link_libraries(mroscore mros_socket)

# ---------------------------- Automated Unit Tests ----------------------------
enable_testing()
add_executable(test_mediator
        test/mediator/test_mediator.cpp
        src/mediator/mediator.cpp
        src/mros/mros.cpp
        src/mros/utils/utils.cpp
)
target_link_libraries(test_mediator GTest::gtest_main mros_socket)
gtest_discover_tests(test_mediator)

add_executable(test_bson_socket test/socket/test_bson_socket.cpp)
target_link_libraries(test_bson_socket GTest::gtest_main mros_socket)
gtest_discover_tests(test_bson_socket)

add_executable(test_bson_rpc_socket test/socket/test_bson_rpc_socket.cpp)
target_link_libraries(test_bson_rpc_socket GTest::gtest_main mros_socket)
gtest_discover_tests(test_bson_rpc_socket)

# ---------------------------- Manual Unit Tests ----------------------------
add_executable(test_manual_node
        test_manual/mros/test_manual_node.cpp
        src/mros/node.cpp
        src/mros/mros.cpp
        src/mros/utils/utils.cpp
)
target_link_libraries(test_manual_node mros_socket)

add_executable(test_manual_publish
        test_manual/mros/test_manual_publish.cpp
        src/mros/node.cpp
        src/mros/mros.cpp
        src/mros/utils/utils.cpp
)
target_link_libraries(test_manual_publish mros_socket)

add_executable(test_manual_subscribe
        test_manual/mros/test_manual_subscribe.cpp
        src/mros/node.cpp
        src/mros/mros.cpp
        src/mros/utils/utils.cpp
)
target_link_libraries(test_manual_subscribe mros_socket)
