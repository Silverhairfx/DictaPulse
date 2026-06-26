set(DICTAPULSE_QML_DIR "${CMAKE_SOURCE_DIR}/qml")

set(DICTAPULSE_QML_FILES
    "${DICTAPULSE_QML_DIR}/Main.qml"
    "${DICTAPULSE_QML_DIR}/Overlay.qml"
    "${DICTAPULSE_QML_DIR}/pages/DashboardPage.qml"
    "${DICTAPULSE_QML_DIR}/pages/ShortcutsPage.qml"
    "${DICTAPULSE_QML_DIR}/pages/ModelsPage.qml"
    "${DICTAPULSE_QML_DIR}/pages/LanguagePage.qml"
    "${DICTAPULSE_QML_DIR}/pages/OutputPage.qml"
    "${DICTAPULSE_QML_DIR}/pages/CleanupPage.qml"
    "${DICTAPULSE_QML_DIR}/pages/ProfilePage.qml"
    "${DICTAPULSE_QML_DIR}/pages/profile/ProfileRefine.qml"
    "${DICTAPULSE_QML_DIR}/pages/profile/ProfilePersonalize.qml"
    "${DICTAPULSE_QML_DIR}/pages/profile/ProfileDevsVoice.qml"
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
    "${DICTAPULSE_QML_DIR}/components/ShortcutCapture.qml"
    "${DICTAPULSE_QML_DIR}/components/AppBackground.qml"
    "${DICTAPULSE_QML_DIR}/components/ClaySurface.qml"
    "${DICTAPULSE_QML_DIR}/components/ClayButton.qml"
    "${DICTAPULSE_QML_DIR}/components/ClaySwitch.qml"
    "${DICTAPULSE_QML_DIR}/components/ClayCheckBox.qml"
    "${DICTAPULSE_QML_DIR}/components/ClaySlider.qml"
    "${DICTAPULSE_QML_DIR}/components/ClaySpinBox.qml"
    "${DICTAPULSE_QML_DIR}/components/ClayComboBox.qml"
    "${DICTAPULSE_QML_DIR}/components/ClayTextField.qml"
    "${DICTAPULSE_QML_DIR}/components/ClayTextArea.qml"
    "${DICTAPULSE_QML_DIR}/components/ClayProgressBar.qml"
    "${DICTAPULSE_QML_DIR}/components/ClayDialog.qml"
    "${DICTAPULSE_QML_DIR}/components/CountUp.qml"
    "${DICTAPULSE_QML_DIR}/components/GlowChart.qml"
)

foreach(qmlfile IN LISTS DICTAPULSE_QML_FILES)
    cmake_path(RELATIVE_PATH qmlfile BASE_DIRECTORY "${DICTAPULSE_QML_DIR}" OUTPUT_VARIABLE rel)
    set_source_files_properties("${qmlfile}" PROPERTIES QT_RESOURCE_ALIAS "${rel}")
endforeach()

set_source_files_properties("${CMAKE_SOURCE_DIR}/resources/icons/dictapulse.svg"
    PROPERTIES QT_RESOURCE_ALIAS icons/dictapulse.svg
)

set_source_files_properties("${CMAKE_SOURCE_DIR}/resources/sounds/pop-in.wav"
    PROPERTIES QT_RESOURCE_ALIAS sounds/pop-in.wav
)
set_source_files_properties("${CMAKE_SOURCE_DIR}/resources/sounds/pop-out.wav"
    PROPERTIES QT_RESOURCE_ALIAS sounds/pop-out.wav
)
set_source_files_properties("${CMAKE_SOURCE_DIR}/resources/sounds/silence.wav"
    PROPERTIES QT_RESOURCE_ALIAS sounds/silence.wav
)

set_source_files_properties("${CMAKE_SOURCE_DIR}/resources/fonts/DMSerifDisplay-Regular.ttf"
    PROPERTIES QT_RESOURCE_ALIAS fonts/DMSerifDisplay-Regular.ttf
)

qt_add_executable(dictapulse
    main.cpp
    app/Settings.h
    app/Settings.cpp
    app/ThemeProvider.h
    app/ThemeProvider.cpp
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
    core/profile/ProfileStats.h
    core/profile/ProfileStats.cpp
    core/profile/ProfileContext.h
    core/profile/ProfileContext.cpp
    core/cleanup/CleanupService.h
    core/cleanup/CleanupService.cpp
    core/cleanup/SecretStore.h
    core/cleanup/SecretStore.cpp
    core/hardware/HardwareInfo.h
    core/hardware/HardwareInfo.cpp
    platform/PlatformAdapter.h
    platform/linux/LinuxAdapter.h
    platform/linux/LinuxAdapter.cpp
    platform/linux/TextInjector.h
    platform/linux/TextInjector.cpp
    platform/linux/ActiveWindowWatcher.h
    platform/linux/ActiveWindowWatcher.cpp
)

target_include_directories(dictapulse PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

qt_add_qml_module(dictapulse
    URI DictaPulse
    VERSION 1.0
    QML_FILES ${DICTAPULSE_QML_FILES}
    RESOURCES
        ${CMAKE_SOURCE_DIR}/resources/icons/dictapulse.svg
        ${CMAKE_SOURCE_DIR}/resources/sounds/pop-in.wav
        ${CMAKE_SOURCE_DIR}/resources/sounds/pop-out.wav
        ${CMAKE_SOURCE_DIR}/resources/sounds/silence.wav
        ${CMAKE_SOURCE_DIR}/resources/fonts/DMSerifDisplay-Regular.ttf
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
    Qt6Keychain::Qt6Keychain
    whisper
)

if(DICTAPULSE_HAVE_KF6)
    target_link_libraries(dictapulse PRIVATE
        KF6::GlobalAccel
        KF6::StatusNotifierItem
        KF6::Notifications
        KF6::ConfigCore
        KF6::ConfigGui
        KF6::ColorScheme
        KF6::WindowSystem
    )
    target_compile_definitions(dictapulse PRIVATE DICTAPULSE_HAVE_KF6)
endif()

# LayerShellQt is the only reliable way to spawn a non-focusing always-on-top
# overlay surface under KWin/Plasma 6 Wayland. Regular QML Window items get
# blocked or invisibly mapped, even with FramelessWindowHint + WindowStaysOnTopHint.
find_package(LayerShellQt QUIET)
if(TARGET LayerShellQt::Interface)
    target_link_libraries(dictapulse PRIVATE LayerShellQt::Interface)
    target_compile_definitions(dictapulse PRIVATE DICTAPULSE_HAVE_LAYERSHELL)
endif()

set_target_properties(dictapulse PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

include(GNUInstallDirs)

install(TARGETS dictapulse RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# Materialize the .desktop file at INSTALL time, not configure time, so the
# Exec path reflects --prefix passed to `cmake --install`. (Previously baked
# /usr/local/bin into the file when the user installed to ~/.local, breaking
# the KDE launcher.)
install(CODE "
    # Exec= must point at the FINAL install location (no DESTDIR), but the file
    # itself is staged under DESTDIR so distro/AppImage packaging works.
    set(_bindir \"\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}\")
    set(_dest_dir \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/applications\")
    file(MAKE_DIRECTORY \"\${_dest_dir}\")
    file(READ \"${CMAKE_SOURCE_DIR}/resources/desktop/dictapulse.desktop.in\" _desktop)
    string(REPLACE \"@CMAKE_INSTALL_FULL_BINDIR@\" \"\${_bindir}\" _desktop \"\${_desktop}\")
    file(WRITE \"\${_dest_dir}/dictapulse.desktop\" \"\${_desktop}\")
    message(STATUS \"Installed desktop entry pointing at \${_bindir}/dictapulse\")
")

install(FILES ${CMAKE_SOURCE_DIR}/resources/icons/dictapulse.svg
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)
