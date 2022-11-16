
file(GLOB_RECURSE SOURCES 
        user/*.c 
        src/*.c 
        src/core/*.c 
        src/draw/*.c 
        src/extra/*.c 
        src/font/*.c 
        src/hal/*.c 
        src/misc/*.c 
        src/widgets/*.c 
        src/extra/layouts/*.c 
        src/extra/layouts/flex/*.c 
        src/extra/layouts/grid/*.c 
        src/extra/themes/*.c 
        src/extra/themes/basic/*.c 
        src/extra/themes/default/*.c 
        src/extra/widgets/calendar/*.c 
        src/extra/widgets/colorwheel/*.c 
        src/extra/widgets/*.c 
        src/extra/widgets/imgbtn/*.c 
        src/extra/widgets/keyboard/*.c 
        src/extra/widgets/led/*.c 
        src/extra/widgets/list/*.c 
        src/extra/widgets/msgbox/*.c 
        src/extra/widgets/spinbox/*.c 
        src/extra/widgets/spinner/*.c 
        src/extra/widgets/tabview/*.c 
        src/extra/widgets/tileview/*.c 
        src/extra/widgets/win/*.c
        )

set(include_dirs . user
src 
src/core 
src/draw 
src/extra 
src/font 
src/hal 
src/misc 
src/widgets 
src/extra/layouts 
src/extra/layouts/flex 
src/extra/layouts/grid 
src/extra/themes 
src/extra/themes/basic 
src/extra/themes/default 
src/extra/widgets/calendar 
src/extra/widgets/colorwheel 
src/extra/widgets 
src/extra/widgets/imgbtn 
src/extra/widgets/keyboard 
src/extra/widgets/led 
src/extra/widgets/list 
src/extra/widgets/msgbox 
src/extra/widgets/spinbox 
src/extra/widgets/spinner 
src/extra/widgets/tabview 
src/extra/widgets/tileview 
src/extra/widgets/win
    )
idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS ${include_dirs}
                    REQUIRES lcd)
