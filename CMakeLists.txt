cmake_minimum_required(VERSION 3.14)

project(SB10-Widget VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Quick)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Quick)

set(PROJECT_SOURCES
    common/mvc/Model.cpp
    common/mvc/Model.h
    common/mvc/View.cpp
    common/mvc/View.h
    common/model/Parameter.cpp
    common/model/Parameter.h
    common/model/EnumParameter.cpp
    common/model/EnumParameter.h
    common/model/NumLimits.h
    common/model/Event.cpp
    common/model/Event.h
    common/VarArray.h
    common/StringTokenizer.h
    common/StringTokenizer.cpp
    common/sync/Synchronized.h
    common/sync/Synchronized.cpp
    main.cpp
    main.h
    qml.qrc
)

string(TOLOWER ${CMAKE_BUILD_TYPE} build_type)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(SB10-Widget
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET SB10-Widget APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        set(ANDROID_PACKAGE_SOURCE_DIR ${PROJECT_SOURCE_DIR}/android-sources CACHE INTERNAL "")
        add_library(SB10-Widget SHARED
            ${PROJECT_SOURCES}
        )
    if (build_type STREQUAL debug)
        file(INSTALL ${CMAKE_CURRENT_SOURCE_DIR}/android-sources/ DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/build-SB10-Widget-Android_Qt_5_15_2_Clang_Multi_Abi-Debug/android-build  FILES_MATCHING PATTERN "*")
    endif()
    if (build_type STREQUAL relwithdebinfo)
        file(INSTALL ${CMAKE_CURRENT_SOURCE_DIR}/android-sources/ DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/build-SB10-Widget-Android_Qt_5_15_2_Clang_Multi_Abi-RelWithDebInfo/android-build  FILES_MATCHING PATTERN "*")
    endif()
    if (build_type STREQUAL release)
        file(INSTALL ${CMAKE_CURRENT_SOURCE_DIR}/android-sources/ DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/build-SB10-Widget-Android_Qt_5_15_2_Clang_Multi_Abi-Release/android-build  FILES_MATCHING PATTERN "*")
    endif()

# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(SB10-Widget
          ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(SB10-Widget
  PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Quick)

set_target_properties(SB10-Widget PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER de.bswalz.sb10widget
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS SB10-Widget
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_import_qml_plugins(SB10-Widget)
    qt_finalize_executable(SB10-Widget)
endif()
