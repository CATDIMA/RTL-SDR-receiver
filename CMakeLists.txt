cmake_minimum_required(VERSION 3.5.0)

project(radar)

add_executable(${PROJECT_NAME} main.cpp functions.cpp)

target_link_libraries(${MY_PROJECT_NAME} fftw3 boost_iostreams SoapySDR)
