cmake_minimum_required(VERSION 3.15)
project(Exporter)

add_executable(Exporter Exporter.cpp)

set_target_properties(Exporter PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

target_compile_options(Exporter PRIVATE
    $<$<CXX_COMPILER_ID:Clang,GNU>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W3 /WX>
)

set(OpenCV_DIR S:/dev/libs/opencv-4.10.0/build)
find_package(OpenCV REQUIRED)

target_sources(Exporter PRIVATE
    Exporter.cpp
)

target_include_directories(Exporter PRIVATE
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(Exporter PRIVATE
    ${OpenCV_LIBS}
)
