# idf_component_register(SRCS "mybeep.c" "lcd_st7735s.c"
#                     INCLUDE_DIRS ".")


file(GLOB_RECURSE SOURCES ./*.c
                            )

set(include_dirs 
    .
    )

if(NOT CMAKE_BUILD_EARLY_EXPANSION)
add_compile_options("-w")

endif()


idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS ${include_dirs} 
                    REQUIRES driver)