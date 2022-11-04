cmake_minimum_required(VERSION 3.8)

project(SAMUEL VERSION 2.1.3-alpha LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

if (WIN32)
    add_subdirectory(./vendor/DirectXTex)
endif()

set(PROJECT_SOURCES
    ./source/core/exportTypes/DDSHeader.cpp
    ./source/core/exportTypes/DDSHeader.h
    ./source/core/exportTypes/OBJ.cpp
    ./source/core/exportTypes/OBJ.h
    ./source/core/exportTypes/PNG.cpp
    ./source/core/exportTypes/PNG.h
    ./source/core/idFileTypes/BIM.cpp
    ./source/core/idFileTypes/BIM.h
    ./source/core/idFileTypes/DECL.cpp
    ./source/core/idFileTypes/DECL.h
    ./source/core/idFileTypes/LWO.cpp
    ./source/core/idFileTypes/LWO.h
    ./source/core/idFileTypes/MD6.cpp
    ./source/core/idFileTypes/MD6.h
    ./source/core/idFileTypes/PackageMapSpec.cpp
    ./source/core/idFileTypes/PackageMapSpec.h
    ./source/core/idFileTypes/ResourceFile.cpp
    ./source/core/idFileTypes/ResourceFile.h
    ./source/core/idFileTypes/StreamDBFile.cpp
    ./source/core/idFileTypes/StreamDBFile.h
    ./source/core/idFileTypes/StreamDBGeometry.cpp
    ./source/core/idFileTypes/StreamDBGeometry.h
    ./source/core/Common.h
    ./source/core/ExportBIM.cpp
    ./source/core/ExportBIM.h
    ./source/core/ExportCOMP.cpp
    ./source/core/ExportCOMP.h
    ./source/core/ExportDECL.cpp
    ./source/core/ExportDECL.h
    ./source/core/ExportModel.cpp
    ./source/core/ExportModel.h
    ./source/core/ExportManager.cpp
    ./source/core/ExportManager.h
    ./source/core/Oodle.cpp
    ./source/core/Oodle.h
    ./source/core/ResourceFileReader.cpp
    ./source/core/ResourceFileReader.h
    ./source/core/SAMUEL.cpp
    ./source/core/SAMUEL.h
    ./source/core/Utilities.cpp
    ./source/core/Utilities.h
    ./source/qt/mainwindow.ui
    ./source/qt/mainwindow.cpp
    ./source/qt/mainwindow.h
    ./source/qt/main.cpp
    ./vendor/jsonxx/jsonxx.cc
    ./vendor/jsonxx/jsonxx.h
)

if (NOT WIN32)
    set(PROJECT_SOURCES ${PROJECT_SOURCES} ./vendor/detex/detex.h)
endif()

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

if (WIN32)
    target_link_libraries(SAMUEL PRIVATE DirectXTex)
else()
    find_package(PNG REQUIRED)
    include_directories(${PNG_INCLUDE_DIR})
    target_link_libraries(SAMUEL PRIVATE ${CMAKE_SOURCE_DIR}/vendor/detex/libdetex.a ${PNG_LIBRARY})
endif()

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
