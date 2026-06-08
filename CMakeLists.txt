cmake_minimum_required(VERSION 3.21)

project(DictaPulse
    VERSION 0.1.1
    DESCRIPTION "Local AI voice dictation for KDE Plasma"
    HOMEPAGE_URL "https://github.com/Silverhairfx/DictaPulse"
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---- Options ----
option(DICTAPULSE_ENABLE_VULKAN "Build whisper.cpp with Vulkan acceleration" OFF)
option(DICTAPULSE_ENABLE_CUDA   "Build whisper.cpp with CUDA acceleration"   OFF)
option(DICTAPULSE_ENABLE_HIP    "Build whisper.cpp with ROCm/HIP acceleration" OFF)

# ---- Qt 6 ----
# 6.9+ for RectangularShadow (QtQuick.Effects), used by the clay design system.
find_package(Qt6 6.9 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Qml
    Quick
    QuickControls2
    Multimedia
    Svg
    Network
    DBus
)

qt_standard_project_setup(REQUIRES 6.9)
qt_policy(SET QTP0004 NEW)

# Secure storage for LLM API keys (KWallet/libsecret on Linux, Credential
# Manager on Windows, Keychain on macOS — same API across all three).
find_package(Qt6Keychain REQUIRED)

# ---- Optional: KDE Frameworks 6 ----
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
            ColorScheme
            WindowSystem
        )
        if(KF6GlobalAccel_FOUND AND KF6StatusNotifierItem_FOUND
           AND KF6Notifications_FOUND AND KF6Config_FOUND
           AND KF6ColorScheme_FOUND AND KF6WindowSystem_FOUND)
            set(DICTAPULSE_HAVE_KF6 TRUE)
            message(STATUS "DictaPulse: KF6 integration enabled")
        else()
            message(STATUS "DictaPulse: KF6 integration disabled (components missing)")
        endif()
    else()
        message(STATUS "DictaPulse: KF6 integration disabled (ECM not found)")
    endif()
endif()

# ---- whisper.cpp (vendored via FetchContent) ----
include(FetchContent)
set(WHISPER_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(WHISPER_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(WHISPER_BUILD_SERVER OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

if(DICTAPULSE_ENABLE_VULKAN)
    set(GGML_VULKAN ON CACHE BOOL "" FORCE)
    message(STATUS "DictaPulse: Vulkan acceleration enabled")
endif()
if(DICTAPULSE_ENABLE_CUDA)
    set(GGML_CUDA ON CACHE BOOL "" FORCE)
    message(STATUS "DictaPulse: CUDA acceleration enabled")
endif()
if(DICTAPULSE_ENABLE_HIP)
    set(GGML_HIP ON CACHE BOOL "" FORCE)
    message(STATUS "DictaPulse: ROCm/HIP acceleration enabled")
endif()

FetchContent_Declare(
    whisper_cpp
    GIT_REPOSITORY https://github.com/ggml-org/whisper.cpp.git
    GIT_TAG        v1.7.6
    GIT_SHALLOW    TRUE
    EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(whisper_cpp)

add_subdirectory(src)
