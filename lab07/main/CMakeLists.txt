set(SOURCE main.c game.c board.c graphics.c nav.c com.c)
idf_component_register(SRCS ${SOURCE}
                       INCLUDE_DIRS .
                       PRIV_REQUIRES esp_timer driver config lcd pin joy)
# target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")
