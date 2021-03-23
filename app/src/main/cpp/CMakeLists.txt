cmake_minimum_required(VERSION 3.10.2)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")

project("opengl3nativerender")

add_library(
        opengl-renderer
        SHARED
        opengl_triangle_renderer.cpp
        #        opengl_renderer.cpp
)

find_library(opengl-lib GLESv3)
find_library(egl-lib EGL)
find_library(android-lib android)
find_library(
        log-lib
        log)
target_link_libraries(
        opengl-renderer
        ${log-lib}
        ${opengl-lib}
        ${egl-lib}
        ${android-lib}
)