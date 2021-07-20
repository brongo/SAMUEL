cmake_minimum_required(VERSION 3.8)
project(SAMUEL)

set(CMAKE_CXX_STANDARD 17)

file(GLOB SOURCES
        ./source/FileExporter.cpp
        ./source/FileExporter.h
        ./source/PackageMapSpec.cpp
        ./source/PackageMapSpec.h
        ./source/ResourceFile.cpp
        ./source/ResourceFile.h
        ./source/SAMUEL.cpp
        ./source/SAMUEL.h
        ./source/StreamDBFile.cpp
        ./source/StreamDBFile.h
        ./source/Utilities.cpp
        ./source/Utilities.h
        ./vendor/jsonxx/jsonxx.cc
        ./vendor/jsonxx/jsonxx.h
        )

if(MSVC)
        set (CMAKE_CXX_FLAGS "/O2 /Oi /Ot /EHsc")
else()
        set (CMAKE_CXX_FLAGS "-Ofast -Wno-unused-result")
endif(MSVC)

add_executable(SAMUEL ${SOURCES})

target_link_libraries(
        SAMUEL
        ${CMAKE_DL_LIBS}
)
