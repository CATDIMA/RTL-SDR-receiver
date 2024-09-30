cmake_minimum_required(VERSION 3.5.0)

set(MY_PROJECT_NAME radar)
project(${MY_PROJECT_NAME})

add_executable(${MY_PROJECT_NAME} main.cpp functions.cpp)

target_link_libraries(${MY_PROJECT_NAME} fftw3 boost_iostreams SoapySDR)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
