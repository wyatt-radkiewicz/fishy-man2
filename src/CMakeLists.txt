cmake_minimum_required(VERSION 3.23.2)
project(fishy-man2 VERSION 0.1)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)
add_executable(fishy-man2)
target_sources(fishy-man2 PRIVATE entities/fishyman.c world.c camera.c game.c entity.c animation.c)

if(WIN32)
    find_package(raylib CONFIG REQUIRED)
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(raylib REQUIRED raylib)
endif()

target_link_libraries(fishy-man2 PRIVATE raylib)

add_library(cLDtk STATIC)
target_sources(cLDtk PRIVATE cLDtk/cLDtk.c cLDtk/parson.c)
target_include_directories(cLDtk PUBLIC cLDtk)

target_link_libraries(fishy-man2 PRIVATE cLDtk)
