qt_add_executable(dictapulse
    main.cpp
)

set_source_files_properties(${CMAKE_SOURCE_DIR}/qml/Main.qml
    PROPERTIES QT_RESOURCE_ALIAS Main.qml
)

qt_add_qml_module(dictapulse
    URI DictaPulse
    VERSION 1.0
    QML_FILES
        ${CMAKE_SOURCE_DIR}/qml/Main.qml
)

target_link_libraries(dictapulse PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Multimedia
    Qt6::Svg
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

install(TARGETS dictapulse
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION bin
)
