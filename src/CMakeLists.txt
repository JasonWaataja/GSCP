cmake_minimum_required(VERSION 3.0)
#project(GSCP)

file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.c)
add_executable(gscp ${SOURCES})

#include_directories(${PROJECT_SOURCE_DIR}/libssh2/include)
#add_subdirectory(${PROJECT_SOURCE_DIR}/libssh2)
#target_link_libraries(gscp libssh2)

find_package(Libssh2 REQUIRED CONFIG)
target_link_libraries(gscp Libssh2::libssh2)

install(TARGETS gscp
   RUNTIME DESTINATION bin)
