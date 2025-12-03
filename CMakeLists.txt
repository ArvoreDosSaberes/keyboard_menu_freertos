add_library(keyboard_menu_freertos STATIC
    keyboard_task.c
    menu_oled_task.c
    menu_event_group.c
)

# Inclui headers deste módulo e do projeto
target_include_directories(keyboard_menu_freertos PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/inc
)

target_link_libraries(keyboard_menu_freertos PUBLIC
    FreeRTOS-Kernel-Heap4
    pico_stdlib
    keyboard
    rack_includes
    log_vt100
    oled
)

# Integrar com FreeRTOS (usa tasks/event groups)
link_freertos(keyboard_menu_freertos)
