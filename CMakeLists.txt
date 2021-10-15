cmake_minimum_required(VERSION 3.20)

project(msdf-atlas-gen)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
if (COMPILER_SUPPORTS_CXX11)
	add_definitions(-DMSDFGEN_USE_CPP11)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

# Make release mode default (turn on optimizations)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

find_package (Threads REQUIRED)

#----------------------------------------------------------------
# Support Functions
#----------------------------------------------------------------

# Mirror the folder structure for sources inside the IDE...
function(folderize_sources sources prefix)
	foreach(FILE ${${sources}}) 
	  get_filename_component(PARENT_DIR "${FILE}" PATH)

	  # skip src or include and changes /'s to \\'s
	  string(REPLACE "${prefix}" "" GROUP "${PARENT_DIR}")
	  string(REPLACE "/" "\\" GROUP "${GROUP}")

      # If it's got a path, then append a "\\" separator (otherwise leave it blank)
	  if ("${GROUP}" MATCHES ".+")
	  	set(GROUP "\\${GROUP}")
	  endif()

	  source_group("${GROUP}" FILES "${FILE}")
	endforeach()
endfunction(folderize_sources)



file(GLOB_RECURSE msdfgen_HEADERS
	msdf-atlas-gen/*.h
	msdf-atlas-gen/*.hpp
)

file(GLOB_RECURSE msdfgen_SOURCES
	msdf-atlas-gen/*.cpp
)

include_directories (
	artery-font-format
    msdfgen
    msdfgen/include
)

add_subdirectory (msdfgen)
add_subdirectory(third_party/fmt)

# Build the library (aliased name because it's the same target name the exe)
folderize_sources(msdf-atlas-gen_HEADERS ${CMAKE_SOURCE_DIR})
folderize_sources(msdf-atlas-gen_SOURCES ${CMAKE_SOURCE_DIR})

add_library(lib_msdf-atlas-gen ${msdfgen_SOURCES} ${msdfgen_HEADERS})
set_target_properties(lib_msdf-atlas-gen PROPERTIES OUTPUT_NAME msdf-atlas-gen)
target_link_libraries(lib_msdf-atlas-gen msdfgen::msdfgen msdfgen::msdfgen-ext fmt::fmt)

# Build the executable

add_executable(msdf-atlas-gen msdf-atlas-gen/main.cpp)
target_link_libraries(msdf-atlas-gen lib_msdf-atlas-gen msdfgen::msdfgen msdfgen::msdfgen-ext ${CMAKE_THREAD_LIBS_INIT})

include(ClangFormat)
target_clangformat_setup(msdf-atlas-gen)
target_clangformat_setup(lib_msdf-atlas-gen)

cmake_minimum_required(VERSION 3.10)

project(msdf-atlas-gen VERSION 1.2 LANGUAGES CXX)
option(MSDF_ATLAS_GEN_BUILD_STANDALONE "Build the msdf-atlas-gen standalone executable" ON)
set(MSDFGEN_BUILD_MSDFGEN_STANDALONE OFF CACHE BOOL "Build the msdfgen standalone executable")
set(MSDFGEN_USE_OPENMP OFF CACHE INTERNAL "Build with OpenMP support for multithreaded code (disabled for atlas gen)" FORCE)
set(MSDFGEN_USE_CPP11 ON CACHE INTERNAL "Build with C++11 enabled (always enabled for atlas gen)" FORCE)
set(MSDFGEN_INSTALL OFF CACHE BOOL "Generate installation target for msdfgen")
add_subdirectory(msdfgen)

find_package(Threads REQUIRED)

file(GLOB_RECURSE msdf-atlas-gen_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    "msdf-atlas-gen/*.h"
    "msdf-atlas-gen/*.hpp"
)

file(GLOB_RECURSE msdf-atlas-gen_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    "msdf-atlas-gen/*.cpp"
)

# msdf-atlas-gen library
add_library(msdf-atlas-gen ${msdf-atlas-gen_SOURCES} ${msdf-atlas-gen_HEADERS})
add_library(msdf-atlas-gen::msdf-atlas-gen ALIAS msdf-atlas-gen)
set_target_properties(msdf-atlas-gen PROPERTIES PUBLIC_HEADER "${msdf-atlas-gen_HEADERS}")
target_include_directories(msdf-atlas-gen
INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/msdfgen/include # for lodepng.h
    ${CMAKE_CURRENT_SOURCE_DIR}/artery-font-format
)

if (MSVC)
    target_compile_definitions(msdf-atlas-gen PUBLIC _CRT_SECURE_NO_WARNINGS)
endif()
target_compile_features(msdf-atlas-gen PUBLIC cxx_std_11)
target_link_libraries(msdf-atlas-gen PUBLIC Threads::Threads msdfgen::msdfgen msdfgen::msdfgen-ext)

# TODO make these public in msdfgen so that this doesn't have to be repeated here
if(FREETYPE_WITH_PNG)
    find_package(ZLIB REQUIRED)
    find_package(PNG REQUIRED)
    target_link_libraries(msdf-atlas-gen PUBLIC ZLIB::ZLIB PNG::PNG)
endif()
if(FREETYPE_WITH_HARFBUZZ)
    # No FindHarfBuzz.cmake available so falling back to CMAKE_LIBRARY_PATH
    find_library(FREETYPE_HARFBUZZ_LIB "harfbuzz")
    target_link_libraries(msdf-atlas-gen PUBLIC "${FREETYPE_HARFBUZZ_LIB}")
endif()

# msdf-atlas-gen standalone executable
if(MSDF_ATLAS_GEN_BUILD_STANDALONE)
    add_executable(msdf-atlas-gen-standalone msdf-atlas-gen/main.cpp)
    set_target_properties(msdf-atlas-gen-standalone PROPERTIES ARCHIVE_OUTPUT_DIRECTORY archive OUTPUT_NAME msdf-atlas-gen)
    target_compile_definitions(msdf-atlas-gen-standalone PUBLIC MSDF_ATLAS_STANDALONE)
    target_compile_features(msdf-atlas-gen-standalone PUBLIC cxx_std_11)
    target_link_libraries(msdf-atlas-gen-standalone PUBLIC msdf-atlas-gen::msdf-atlas-gen)
endif()
