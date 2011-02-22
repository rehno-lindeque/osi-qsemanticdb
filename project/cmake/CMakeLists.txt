##############################################################################
#
#     CMAKELISTS.TXT
#
#     Copyright (c) 2011 Rehno Lindeque. All rights reserved.
#
##############################################################################

cmake_minimum_required(VERSION 2.6) 

# Cacheable options
if(DEFINED CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "Choose the type of build, options are: None Debug Release.")
else()
   set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build, options are: None Debug Release.")
endif()
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "../../lib" CACHE STRING "The directory where static libraries will be placed.")
set(SEMANTICDB_API_LANGUAGE C CACHE STRING "Choose the export language to use, options are: CPP C.")
set(SEMANTICDB_SHARED_LIB OFF CACHE BOOL "If this flag is turned on then a shared library will be created instead of a static one.")

project(qsemanticdb)

# Build flags

set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_FLAGS_DEBUG  "-g -D_DEBUG") 
if(SEMANTICDB_API_LANGUAGE STREQUAL C)
  if(SEMANTICDB_SHARED_LIB)
    add_definitions(-DOSI_C_SHARED_BUILD)
  else()
    add_definitions(-DOSI_C_STATIC_BUILD)
  endif()
else()
  if(SEMANTICDB_SHARED_LIB)
    add_definitions(-DOSI_CPP_SHARED_BUILD)
  else()
    add_definitions(-DOSI_CPP_STATIC_BUILD)
  endif()
endif()


# Build

if(CMAKE_BUILD_TYPE STREQUAL Debug)
  if(SEMANTICDB_API_LANGUAGE STREQUAL C)
    set(SEMANTICDB_LIBRARY_NAME qsemanticdbcd)
  else()
    set(SEMANTICDB_LIBRARY_NAME qsemanticdbd)
  endif()
else() 
  if(SEMANTICDB_API_LANGUAGE STREQUAL C)
    set(SEMANTICDB_LIBRARY_NAME qsemanticdbc)
  else()
    set(SEMANTICDB_LIBRARY_NAME qsemanticdb)
  endif()
endif()

include_directories(../../src  ../../../../api ../../../base/src ../../../basesemanticdb/src)
set_source_files_properties(../../src/api.cpp COMPILE_FLAGS "-std=c++0x -Wall")
add_library(${SEMANTICDB_LIBRARY_NAME} STATIC ../../src/api.cpp)
