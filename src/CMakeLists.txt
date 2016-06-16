cmake_minimum_required(VERSION 3.0)
#project(GSCP)

file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.c)
#add_executable(gscp ${SOURCES} resources.c)
add_executable(gscp ${SOURCES})

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
# Setup CMake to use GTK+, tell the compiler where to look for headers
# and to the linker where to look for libraries
include_directories(${GTK3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})
# Add other flags to the compiler
add_definitions(${GTK3_CFLAGS_OTHER})
# Add an executable compiled from hello.c
#add_executable(hello main.c)
# Link the target to the GTK+ libraries
target_link_libraries(gscp ${GTK3_LIBRARIES})

#for glib compile I think

#add_custom_command(TARGET gscp
  #PRE_BUILD
  #COMMAND sh -c '`pkg-config --variable=glib_compile_resources gio-2.0` gscp.gresource.xml --target=resources.c --sourcedir=. --generate-source'
  #)

#include_directories(${PROJECT_SOURCE_DIR}/libssh2/include)
#add_subdirectory(${PROJECT_SOURCE_DIR}/libssh2)
#target_link_libraries(gscp libssh2)

find_package(Libssh2 REQUIRED CONFIG)
target_link_libraries(gscp Libssh2::libssh2)

install(TARGETS gscp
   RUNTIME DESTINATION bin)
