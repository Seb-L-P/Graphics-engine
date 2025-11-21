cmake_minimum_required(VERSION 3.10)
project(MyGraphicsEngine)

set(CMAKE_CXX_STANDARD 17)

# 1. Find Packages
find_package(glfw3 3.3 REQUIRED)
find_package(PkgConfig REQUIRED)

# Find Assimp using PkgConfig
pkg_check_modules(ASSIMP REQUIRED assimp)

# --- CRITICAL FIX: Tell linker where to find the library files ---
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