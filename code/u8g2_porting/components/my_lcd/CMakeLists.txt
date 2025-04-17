# idf_component_register(SRCS "mylcd_draw.c" "mylcd.c" "lcd_st7789_190_wlk.c" "lcd_st7735s_096.c" "lcd_st7789_114.c" "lcd_st7789_190.c" "lcd_st7735s.c"
#                     INCLUDE_DIRS ".")


file(GLOB_RECURSE SOURCES ./*.c
                            ./my_lcd_font/*.c
                            )

set(include_dirs 
    .
    ./my_lcd_font
    )

if(NOT CMAKE_BUILD_EARLY_EXPANSION)
add_compile_options("-w")

endif()


idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS ${include_dirs} 
                    REQUIRES driver)