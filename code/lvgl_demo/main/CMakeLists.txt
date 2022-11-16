# idf_component_register(SRCS "*.c"
#                     INCLUDE_DIRS ".")


file(GLOB_RECURSE SOURCES ./*.c)

set(include_dirs 
    .
    )
idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS ${include_dirs})
