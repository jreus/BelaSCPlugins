# See: https://cmake.org/cmake-tutorial/ for getting started with cmake
# Build with
#

# cmake -DSC_PATH=~/supercollider3.10b/ -DBELA_PATH=~/Bela/ -DCMAKE_C_COMPILE=/usr/bin/gcc-6 -DCMAKE_CXX_COMPILE=/usr/bin/g++-6 -DCMAKE_C_FLAGS="-march=armv7-a -mtune=cortex-a8 -mfloat-abi=hard -mfpu=neon -O2" -DCMAKE_CPP_FLAGS="-march=armv7-a -mtune=cortex-a8 -mfloat-abi=hard -mfpu=neon -O2" ..

# cmake -DCMAKE_BUILD_TYPE=RELEASE ..

# make

set(BELA_PATH /root/Bela/)
set(CMAKE_BUILD_TYPE Release)
message(STATUS "BELA PATH IS ${BELA_PATH}")


cmake_minimum_required (VERSION 2.8)
project(trillin)


# Include all the search directories
include_directories(${BELA_PATH}/include)
include_directories(${BELA_PATH}/lib)
include_directories(${BELA_PATH}/libraries/Trill/)
include_directories(${BELA_PATH}/libraries/OSCSender/)

# Build the shared library
add_executable(hello
  trillin.cpp
  ${BELA_PATH}/libraries/OSCSender/OSCSender.cpp
  ${BELA_PATH}/libraries/Trill/Trill.cpp
)
