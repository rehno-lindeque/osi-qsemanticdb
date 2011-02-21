##############################################################################
#
#     CMAKELISTS.TXT
#
#     Copyright (c) 2011 Rehno Lindeque. All rights reserved.
#
##############################################################################

cmake_minimum_required(VERSION 2.6) 

# Define build types
if(DEFINED CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "Choose the type of build, options are: None Debug Release.")
else()
   set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build, options are: None Debug Release.")
endif()

project(qsemanticdb)

# Options

set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_FLAGS_DEBUG  "-g -D_DEBUG") 

# Build

include_directories(../../src  ../../../../api ../../../base/src ../../../basesemanticdb/src)
set_source_files_properties(../../src/api.cpp COMPILE_FLAGS "-std=c++0x -Wall")

if(CMAKE_BUILD_TYPE STREQUAL Debug)
   set(SEMANTICDB_LIBRARY_NAME qsemanticdbd)
else() 
   set(SEMANTICDB_LIBRARY_NAME qsemanticdb) 
endif()

add_library(${SEMANTICDB_LIBRARY_NAME} STATIC ../../src/api.cpp)
