cmake_minimum_required(VERSION 3.8)

project(SAMUEL VERSION 2.1.2 LANGUAGES CXX)

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
endif ()

set(PROJECT_SOURCES
        ./source/core/exportTypes/DDSHeader.cpp
#        ./source/core/exportTypes/OBJ.cpp
        ./source/core/exportTypes/PNG.cpp
        ./source/core/exportTypes/CAST.cpp
        ./source/core/idFileTypes/BIM.cpp
        ./source/core/idFileTypes/DECL.cpp
        #        ./source/core/idFileTypes/LWO.cpp
        ./source/core/idFileTypes/MD6Mesh.cpp
        ./source/core/idFileTypes/PackageMapSpec.cpp
        ./source/core/idFileTypes/ResourceFile.cpp
        ./source/core/idFileTypes/StreamDBFile.cpp
        ./source/core/idFileTypes/StreamDBGeometry.cpp
        ./source/core/ExportBIM.cpp
        #        ./source/core/ExportCOMP.cpp
        ./source/core/ExportDECL.cpp
        ./source/core/ExportModel.cpp
        ./source/core/ExportManager.cpp
        ./source/core/Oodle.cpp
        ./source/core/SAMUEL.cpp
        ./source/core/Utilities.cpp
        ./source/core/Utilities.h
        ./vendor/jsonxx/jsonxx.cc
        ./source/core/cast/cast.cpp
        ./source/core/ResourceManager.cpp
        source/core/idFileTypes/MD6Skl.cpp

        )

set(PROJECT_SOURCES_QT
        ./source/qt/mainwindow.ui
        ./source/qt/mainwindow.cpp
        ./source/qt/main.cpp
        )

if (NOT WIN32)
    set(PROJECT_SOURCES ${PROJECT_SOURCES} ./vendor/detex/detex.h)
endif ()

set(APP_ICON_RESOURCE_WINDOWS "./resources/icon.rc" source/core/ResourceManager.cpp source/core/ResourceManager.h)

add_executable(SAMUEL_cli
        ./source/core/ResourceManager.cpp
        ./source/core/Oodle.cpp
        ./source/core/Utilities.cpp
        ./source/core/idFileTypes/ResourceFile.cpp
        ./source/core/idFileTypes/PackageMapSpec.cpp
        ./source/core/idFileTypes/StreamDBFile.cpp
        ./source/core/idFileTypes/StreamDBGeometry.cpp
        ./source/core/idFileTypes/MD6Mesh.cpp
        ./source/core/idFileTypes/MD6Skl.cpp
        ./source/core/idFileTypes/BIM.cpp
        ./source/core/idFileTypes/DECL.cpp
        ./source/core/ExportModel.cpp
        ./source/core/ExportBIM.cpp
        ./source/core/ExportDECL.cpp
        ./source/core/ExportManager.cpp
        ./source/core/exportTypes/CAST.cpp
        ./source/core/exportTypes/DDSHeader.cpp
        ./source/core/exportTypes/PNG.cpp
        ./source/core/cast/cast.cpp
        ./vendor/jsonxx/jsonxx.cc
        ./source/cli/main.cpp
        )

if (${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(SAMUEL
            WIN32
            MANUAL_FINALIZATION
            ${PROJECT_SOURCES}
            ${PROJECT_SOURCES_QT}
            ${APP_ICON_RESOURCE_WINDOWS}
            )
else ()
    if (ANDROID)
        add_library(SAMUEL SHARED
                ${PROJECT_SOURCES}
                ${PROJECT_SOURCES_QT}
                )
    else ()
        add_executable(SAMUEL
                ${PROJECT_SOURCES}
                ${PROJECT_SOURCES_QT}
                ${APP_ICON_RESOURCE_WINDOWS}
                )
    endif ()
endif ()

target_link_libraries(SAMUEL PRIVATE Qt${QT_VERSION_MAJOR}::Widgets ${CMAKE_DL_LIBS})

if (WIN32)
    target_link_libraries(SAMUEL PRIVATE DirectXTex)
    target_link_libraries(SAMUEL_cli PRIVATE DirectXTex)
    find_program(WINDEPLOYQT_EXECUTABLE NAMES windeployqt HINTS ${QTDIR} ENV QTDIR PATH_SUFFIXES bin)
    add_custom_command(TARGET SAMUEL POST_BUILD
            COMMAND ${WINDEPLOYQT_EXECUTABLE} --no-translations $<TARGET_FILE_DIR:SAMUEL>)
else ()
    find_package(PNG REQUIRED)
    include_directories(${PNG_INCLUDE_DIR})
    target_link_libraries(SAMUEL PRIVATE ${CMAKE_SOURCE_DIR}/vendor/detex/libdetex.a ${PNG_LIBRARY})
endif ()

if (MSVC)
    set(CMAKE_CXX_FLAGS "/O2 /Oi /Ot /EHsc")
else ()
    set(CMAKE_CXX_FLAGS "-Ofast -Wno-unused-result -pthread")
endif ()

set_target_properties(SAMUEL PROPERTIES
        MACOSX_BUNDLE_GUI_IDENTIFIER io.github.samuel
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        )

if (QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(SAMUEL)
endif ()
