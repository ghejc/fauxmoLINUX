cmake_minimum_required(VERSION 3.16)
project(fauxmoLINUX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_LIBRARY_PATH /usr/lib/arm-linux-gnueabihf)

# Using GCC 7.1 or later, you can safely ignore the warning
# "parameter passing for X changed in GCC 7.1"
if(LINUX)
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-psabi")
endif()

add_executable(fauxmoLINUX src/fauxmoLINUX.cpp src/remote_control.cpp)

# Add -lpthread
find_library(NAMES pthread)
target_link_libraries(fauxmoLINUX pthread)

# Add -lcrypto
find_library(NAMES crypto)
target_link_libraries(fauxmoLINUX crypto)

# Add -I../async-sockets
target_include_directories(fauxmoLINUX PUBLIC ./src ./async-sockets-cpp/async-sockets/include ./json/include)

# Add boost libraries
set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 
find_package(Boost 1.71.0 COMPONENTS system) 

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS}) 
    target_link_libraries(fauxmoLINUX ${Boost_LIBRARIES})
endif()