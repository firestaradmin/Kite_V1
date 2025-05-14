

file(GLOB SOURCES 
    ./main.c
    ./user/ble_server.c
    ./drv/led/*.c
    ./drv/button/*.c
)

set(include_dirs  
    .
    ./drv/led 
    ./drv/button
)


idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS ${include_dirs})

target_compile_options(${COMPONENT_LIB} PRIVATE -w -Wno-unused-const-variable)
