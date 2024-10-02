cmake_minimum_required(VERSION 3.15)
project(Exporter)

add_executable(Exporter Exporter.cpp)

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
