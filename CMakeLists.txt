cmake_minimum_required(VERSION 3.10)
project(MyGraphicsEngine)

set(CMAKE_CXX_STANDARD 17)

# 1. Find Packages
find_package(glfw3 3.3 REQUIRED)
find_package(PkgConfig REQUIRED)

# Find Assimp
pkg_check_modules(ASSIMP REQUIRED assimp)
link_directories(${ASSIMP_LIBRARY_DIRS})

# 2. Setup Executable
# We added all the ImGui .cpp files here
add_executable(MyGraphicsEngine 
    main.cpp 
    src/glad.c 
    src/stb_image_impl.cpp
    src/imgui/imgui.cpp
    src/imgui/imgui_demo.cpp
    src/imgui/imgui_draw.cpp
    src/imgui/imgui_tables.cpp
    src/imgui/imgui_widgets.cpp
    src/imgui/imgui_impl_glfw.cpp
    src/imgui/imgui_impl_opengl3.cpp
)

# 3. Include Directories
# We added include/imgui so we can just type #include "imgui.h"
target_include_directories(MyGraphicsEngine PRIVATE 
    include 
    include/imgui
    ${ASSIMP_INCLUDE_DIRS} 
)

# 4. Link Libraries
target_link_libraries(MyGraphicsEngine 
    glfw 
    ${GLFW_LIBRARIES}
    ${ASSIMP_LIBRARIES}
)

if (UNIX AND NOT APPLE)
    target_link_libraries(MyGraphicsEngine pthread dl)
endif()

# 5. Auto-Copy Assets
file(GLOB ASSETS
    "${CMAKE_SOURCE_DIR}/*.vert"
    "${CMAKE_SOURCE_DIR}/*.frag"
    "${CMAKE_SOURCE_DIR}/*.obj"
    "${CMAKE_SOURCE_DIR}/*.mtl"
    "${CMAKE_SOURCE_DIR}/*.jpg"
    "${CMAKE_SOURCE_DIR}/*.png"
)

if(ASSETS)
    file(COPY ${ASSETS} DESTINATION ${CMAKE_BINARY_DIR})
endif()