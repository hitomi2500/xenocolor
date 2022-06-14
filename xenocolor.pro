TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        main.c \
        quantize.c \
        tga.c \
        tga_write.c

HEADERS += \
    main.h \
    quantize.h \
    tga.h \
    tga_write.h
