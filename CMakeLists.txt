cmake_minimum_required(VERSION 3.8)

project(SAMUEL VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

set(PROJECT_SOURCES
        ./source/core/DDSHeader.cpp
        ./source/core/DDSHeader.h
        ./source/core/FileExporter.cpp
        ./source/core/FileExporter.h
        ./source/core/PackageMapSpec.cpp
        ./source/core/PackageMapSpec.h
        ./source/core/ResourceFile.cpp
        ./source/core/ResourceFile.h
        ./source/core/ResourceFileReader.cpp
        ./source/core/ResourceFileReader.h
        ./source/core/SAMUEL.cpp
        ./source/core/SAMUEL.h
        ./source/core/StreamDBFile.cpp
        ./source/core/StreamDBFile.h
        ./source/core/Utilities.cpp
        ./source/core/Utilities.h
        ./source/qt/mainwindow.ui
        ./source/qt/mainwindow.cpp
        ./source/qt/mainwindow.h
        ./source/qt/main.cpp
        ./vendor/jsonxx/jsonxx.cc
        ./vendor/jsonxx/jsonxx.h

)

set(APP_ICON_RESOURCE_WINDOWS "./resources/icon.rc")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(SAMUEL
        WIN32
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ${APP_ICON_RESOURCE_WINDOWS}
    )
else()
    if(ANDROID)
        add_library(SAMUEL SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(SAMUEL
            ${PROJECT_SOURCES}
            ${APP_ICON_RESOURCE_WINDOWS}
        )
    endif()
endif()

target_link_libraries(SAMUEL PRIVATE Qt${QT_VERSION_MAJOR}::Widgets ${CMAKE_DL_LIBS})

if (MSVC)
    set(CMAKE_CXX_FLAGS "/O2 /Oi /Ot /EHsc")
else()
    set(CMAKE_CXX_FLAGS "-Ofast -Wno-unused-result -pthread")
endif()

set_target_properties(SAMUEL PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER io.github.samuel
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(SAMUEL)
endif()
