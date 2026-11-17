find_package(Qt6 6.9 REQUIRED COMPONENTS Test)

function(dictapulse_add_test name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} PRIVATE dictapulse_logic Qt6::Test)
    add_test(NAME ${name} COMMAND ${name})
endfunction()

dictapulse_add_test(tst_insertion)
dictapulse_add_test(tst_hyprland_ipc)
dictapulse_add_test(tst_input_device)
dictapulse_add_test(tst_control_channel)

# Live insertion probe for scripts that run inside a disposable compositor.
# Not registered with ctest: it needs a running Wayland session.
find_package(Qt6 6.9 REQUIRED COMPONENTS Gui DBus)
add_executable(dp_inject_probe
    live/dp_inject_probe.cpp
    ${CMAKE_SOURCE_DIR}/src/platform/linux/TextInjector.h
    ${CMAKE_SOURCE_DIR}/src/platform/linux/TextInjector.cpp
)
target_link_libraries(dp_inject_probe PRIVATE dictapulse_logic Qt6::Gui Qt6::DBus)
