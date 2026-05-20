set(DICTAPULSE_QML_DIR "${CMAKE_SOURCE_DIR}/qml")

set(DICTAPULSE_QML_FILES
    "${DICTAPULSE_QML_DIR}/Main.qml"
    "${DICTAPULSE_QML_DIR}/Overlay.qml"
    "${DICTAPULSE_QML_DIR}/Theme.qml"
    "${DICTAPULSE_QML_DIR}/pages/DashboardPage.qml"
    "${DICTAPULSE_QML_DIR}/pages/ShortcutsPage.qml"
    "${DICTAPULSE_QML_DIR}/pages/ModelsPage.qml"
    "${DICTAPULSE_QML_DIR}/pages/LanguagePage.qml"
    "${DICTAPULSE_QML_DIR}/pages/OutputPage.qml"
    "${DICTAPULSE_QML_DIR}/pages/BackendPage.qml"
    "${DICTAPULSE_QML_DIR}/pages/OverlayAppearancePage.qml"
    "${DICTAPULSE_QML_DIR}/pages/PrivacyPage.qml"
    "${DICTAPULSE_QML_DIR}/pages/AdvancedPage.qml"
    "${DICTAPULSE_QML_DIR}/pages/AboutPage.qml"
    "${DICTAPULSE_QML_DIR}/components/Waveform.qml"
    "${DICTAPULSE_QML_DIR}/components/StatusPill.qml"
    "${DICTAPULSE_QML_DIR}/components/SectionCard.qml"
    "${DICTAPULSE_QML_DIR}/components/SettingRow.qml"
    "${DICTAPULSE_QML_DIR}/components/SidebarItem.qml"
)

foreach(qmlfile IN LISTS DICTAPULSE_QML_FILES)
    cmake_path(RELATIVE_PATH qmlfile BASE_DIRECTORY "${DICTAPULSE_QML_DIR}" OUTPUT_VARIABLE rel)
    set_source_files_properties("${qmlfile}" PROPERTIES QT_RESOURCE_ALIAS "${rel}")
endforeach()

set_source_files_properties("${DICTAPULSE_QML_DIR}/Theme.qml"
    PROPERTIES
        QT_QML_SINGLETON_TYPE TRUE
        QT_RESOURCE_ALIAS Theme.qml
)

set_source_files_properties("${CMAKE_SOURCE_DIR}/resources/icons/dictapulse.svg"
    PROPERTIES QT_RESOURCE_ALIAS icons/dictapulse.svg
)

qt_add_executable(dictapulse
    main.cpp
    app/Settings.h
    app/Settings.cpp
    app/Controller.h
    app/Controller.cpp
    core/audio/AudioCapture.h
    core/audio/AudioCapture.cpp
    core/transcription/WhisperEngine.h
    core/transcription/WhisperEngine.cpp
    core/models/ModelCatalog.h
    core/models/ModelCatalog.cpp
    core/models/ModelManager.h
    core/models/ModelManager.cpp
    core/text/TextProcessor.h
    core/text/TextProcessor.cpp
    core/hardware/HardwareInfo.h
    core/hardware/HardwareInfo.cpp
    platform/PlatformAdapter.h
    platform/linux/LinuxAdapter.h
    platform/linux/LinuxAdapter.cpp
    platform/linux/TextInjector.h
    platform/linux/TextInjector.cpp
)

target_include_directories(dictapulse PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

qt_add_qml_module(dictapulse
    URI DictaPulse
    VERSION 1.0
    QML_FILES ${DICTAPULSE_QML_FILES}
    RESOURCES
        ${CMAKE_SOURCE_DIR}/resources/icons/dictapulse.svg
)

target_link_libraries(dictapulse PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Multimedia
    Qt6::Svg
    Qt6::Network
    Qt6::DBus
    whisper
)

if(DICTAPULSE_HAVE_KF6)
    target_link_libraries(dictapulse PRIVATE
        KF6::GlobalAccel
        KF6::StatusNotifierItem
        KF6::Notifications
        KF6::ConfigCore
        KF6::ConfigGui
    )
    target_compile_definitions(dictapulse PRIVATE DICTAPULSE_HAVE_KF6)
endif()

set_target_properties(dictapulse PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

include(GNUInstallDirs)

# Substitute the absolute bin path into the .desktop Exec line so KDE/GNOME
# launchers find the binary regardless of whether ~/.local/bin is on the
# graphical session PATH.
configure_file(
    "${CMAKE_SOURCE_DIR}/resources/desktop/dictapulse.desktop.in"
    "${CMAKE_BINARY_DIR}/dictapulse.desktop"
    @ONLY
)

install(TARGETS dictapulse RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${CMAKE_BINARY_DIR}/dictapulse.desktop
        DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(FILES ${CMAKE_SOURCE_DIR}/resources/icons/dictapulse.svg
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)
