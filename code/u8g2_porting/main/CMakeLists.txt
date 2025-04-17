



set(MAIN_SRCS     
    ./main.c
    )


message("========== TEST TEST !!! BUILD ESP32 for KiteV1")
idf_component_register(SRCS 
                            "main.c"
                    INCLUDE_DIRS "."
                                )
target_compile_options(${COMPONENT_LIB} PRIVATE -w -Wno-unused-const-variable)