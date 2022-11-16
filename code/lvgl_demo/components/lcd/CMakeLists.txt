# idf_component_register(SRCS "lcd_st7735s.c"
#                     INCLUDE_DIRS ".")


# 
file(GLOB_RECURSE SOURCES ./*.c)

set(include_dirs 
    .
    )
idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS ${include_dirs})