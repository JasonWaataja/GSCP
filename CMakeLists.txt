cmake_minimum_required(VERSION 3.0)
project(GSCP)

include(ExternalProject)

ExternalProject_Add(
   Libssh2
   PREFIX libssh2
   GIT_REPOSITORY https://github.com/libssh2/libssh2
   INSTALL_COMMAND "")

ExternalProject_Add(
   GSCP DEPENDS Libssh2
   PREFIX GSCP
   SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src
   INSTALL_COMMAND "")

#file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.c)
#add_executable(gscp ${SOURCES})

#include_directories(${PROJECT_SOURCE_DIR}/libssh2/include)
#add_subdirectory(${PROJECT_SOURCE_DIR}/libssh2)
#target_link_libraries(gscp libssh2)

#find_package(Libssh2 REQUIRED CONFIG)
#target_link_libraries(gscp Libssh2::libssh2)
