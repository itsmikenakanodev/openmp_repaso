cmake_minimum_required(VERSION 3.26)
project(openmp_repaso)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fopenmp")

find_package(fmt)
find_package(SFML)

add_executable(rotar rotacion.cpp)

target_link_libraries(rotar fmt::fmt sfml::sfml)

add_executable(bordes bordes.cpp)

target_link_libraries(bordes fmt::fmt sfml::sfml)

add_executable(mediana mediana.cpp)

target_link_libraries(mediana fmt::fmt sfml::sfml)
