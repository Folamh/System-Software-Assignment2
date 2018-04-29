cmake_minimum_required(VERSION 3.10)
project(IntranetFileTransfer C)

set(CMAKE_C_STANDARD 99)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread -lcrypt")

add_executable(IntranetFileTransfer server/main.c server/connection_handler.h server/daemonize.h)

add_executable(IntranetFileTransferClient client/main.c)