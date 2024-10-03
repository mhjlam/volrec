cmake_minimum_required(VERSION 3.5)

# Set project name
project(volrec)

# Set output directory relative to build directory
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Set executable name
add_executable(volrec)

# C++ Standard
set_target_properties(volrec PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Compiler flags
target_compile_options(volrec PRIVATE
    $<$<CXX_COMPILER_ID:Clang,GNU>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W3 /WX>
)

# OpenGL
find_package(OpenGL REQUIRED)

# OpenCV
set(OpenCV_STATIC ON)
set(OpenCV_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/opencv-4.10.0)
find_package(OpenCV REQUIRED)

target_sources(volrec PRIVATE
    src/Main.cpp src/Camera.cpp src/Scene.cpp src/VoxelGrid.cpp
)

# FreeGLUT
set(FREEGLUT_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(FREEGLUT_BUILD_DEMOS OFF CACHE BOOL "" FORCE)
set(FreeGLUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/freeglut-3.6.0)
add_subdirectory(${FreeGLUT_DIR})

# Include directories
target_include_directories(volrec PRIVATE
    ${FreeGLUT_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
)

# Library directories
target_link_libraries(volrec PRIVATE
    freeglut_static
    ${OpenCV_LIBS}
)

# Copy OpenCV binaries to output directory
file(GLOB OPENCV_DLLS "${OpenCV_DIR}/x64/vc17/bin/*.dll")
foreach(DLL IN LISTS OPENCV_DLLS)
    add_custom_command(TARGET volrec POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DLL} $<TARGET_FILE_DIR:volrec>)
endforeach()

# Copy resource files to output directory
add_custom_command(TARGET volrec POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/res" $<TARGET_FILE_DIR:volrec>
)

# CTests
enable_testing()
add_test(NAME Pear COMMAND $<TARGET_FILE:volrec> "$<TARGET_FILE_DIR:volrec>/pear")
add_test(NAME Tape COMMAND $<TARGET_FILE:volrec> "$<TARGET_FILE_DIR:volrec>/tape")
