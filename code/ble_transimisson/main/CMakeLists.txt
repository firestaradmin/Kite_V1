

file(GLOB_RECURSE SOURCES 
    ./main.c
    ./ble_server.c
)

set(include_dirs  . )


idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS ${include_dirs})

target_compile_options(${COMPONENT_LIB} PRIVATE -w -Wno-unused-const-variable)
