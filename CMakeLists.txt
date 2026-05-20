cmake_minimum_required(VERSION 3.21)

project(DictaPulse
    VERSION 0.0.1
    DESCRIPTION "Local AI voice dictation for KDE Plasma"
    HOMEPAGE_URL "https://github.com/Silverhairfx/DictaPulse"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---- Qt 6 ----
find_package(Qt6 6.6 REQUIRED COMPONENTS
    Core
    Gui
    Qml
    Quick
    QuickControls2
    Multimedia
    Svg
)

qt_standard_project_setup(REQUIRES 6.6)

# ---- Optional: KDE Frameworks 6 (Linux only) ----
set(DICTAPULSE_HAVE_KF6 FALSE)
if(UNIX AND NOT APPLE)
    find_package(ECM 6.0 QUIET NO_MODULE)
    if(ECM_FOUND)
        list(APPEND CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
        find_package(KF6 6.0 QUIET COMPONENTS
            GlobalAccel
            StatusNotifierItem
            Notifications
            Config
        )
        if(KF6GlobalAccel_FOUND AND KF6StatusNotifierItem_FOUND
           AND KF6Notifications_FOUND AND KF6Config_FOUND)
            set(DICTAPULSE_HAVE_KF6 TRUE)
            message(STATUS "DictaPulse: KF6 integration enabled")
        else()
            message(STATUS "DictaPulse: KF6 integration disabled (components missing)")
        endif()
    else()
        message(STATUS "DictaPulse: KF6 integration disabled (ECM not found)")
    endif()
endif()

add_subdirectory(src)
