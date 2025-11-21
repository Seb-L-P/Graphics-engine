cmake_minimum_required(VERSION 3.10)
project(MyGraphicsEngine)

set(CMAKE_CXX_STANDARD 17)

# 1. Find Packages
find_package(glfw3 3.3 REQUIRED)
find_package(PkgConfig REQUIRED)

# Find Assimp using PkgConfig
pkg_check_modules(ASSIMP REQUIRED assimp)

# Tell linker where to find the library files
link_directories(${ASSIMP_LIBRARY_DIRS})

# 2. Setup Executable
add_executable(MyGraphicsEngine 
    main.cpp 
    src/glad.c 
    src/stb_image_impl.cpp
)

# 3. Include Directories
target_include_directories(MyGraphicsEngine PRIVATE 
    include 
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

# --- 5. AUTOMATIC ASSET COPYING (NEW) ---
# This finds all asset files in your project root and copies them to the build folder
# so the executable can find them at runtime.
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
    message(STATUS "Auto-Copied assets (shaders/models) to build directory.")
else()
    message(WARNING "No assets found in root directory to copy!")
endif()