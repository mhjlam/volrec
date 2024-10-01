cmake_minimum_required(VERSION 3.15)
project(VolRec)

include(CMakePrintHelpers)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(VolRec)

set_target_properties(VolRec PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

target_compile_options(VolRec PRIVATE
    $<$<CXX_COMPILER_ID:Clang,GNU>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W3 /WX>
)

find_package(OpenGL REQUIRED)
set(OpenCV_STATIC ON)
set(OpenCV_DIR S:/dev/libs/opencv-4.10.0/build)
find_package(OpenCV REQUIRED)

target_sources(VolRec PRIVATE
    src/Main.cpp src/Camera.cpp src/Scene.cpp src/VoxelGrid.cpp
)

set(FREEGLUT_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(FREEGLUT_BUILD_DEMOS OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/freeglut)

target_include_directories(VolRec PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/freeglut/include
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(VolRec PRIVATE
    freeglut_static
    ${OpenCV_LIBS}
)

add_custom_command(TARGET VolRec POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/bin/Debug"
        $<TARGET_FILE_DIR:VolRec>)

add_custom_command(TARGET VolRec POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/res"
        $<TARGET_FILE_DIR:VolRec>)

enable_testing()
add_test(NAME Pear COMMAND $<TARGET_FILE:VolRec> "pear")
add_test(NAME Tape COMMAND $<TARGET_FILE:VolRec> "tape")
