cmake_minimum_required(VERSION 3.0)
project(libssh2test)

file(GLOB SOURCES "./*.c")
add_executable(gscp ${SOURCES})

#find_package(Libssh2 REQUIRED CONFIG)
#target_link_libraries(libssh2test Libssh2::libssh2)

include_directories(${PROJECT_SOURCE_DIR}/libssh2/include)
add_subdirectory(${PROJECT_SOURCE_DIR}/libssh2)
target_link_libraries(gscp libssh2)
