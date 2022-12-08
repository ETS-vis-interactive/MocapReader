cmake_minimum_required(VERSION 3.15)

project(MocapReader)

# Setup language requirements
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#--------------------------------------------------
# Define math lib (header-only library)
#--------------------------------------------------
set( MocapReaderSrc BVHFormat.cpp HTRFormat.cpp Extras.cpp )
set( MocapReaderHdr MocapData.h BVHFormat.h HTRFormat.h Extras.h )
add_library(MocapReaderLib ${MocapReaderSrc} ${MocapReaderHdr} )
target_include_directories(MocapReaderLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

#--------------------------------------------------
# Define test executable
#--------------------------------------------------
add_executable( MocapReaderLibTest MocapReaderLibTest.cpp )
target_link_libraries( MocapReaderLibTest MocapReaderLib)