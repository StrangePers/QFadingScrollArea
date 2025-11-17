QT += core widgets

CONFIG += c++17

TARGET = QFadingScrollAreaExample
TEMPLATE = app

SOURCES += \
    QFadingScrollArea.cpp \
    main.cpp

HEADERS += \
    QFadingScrollArea.h

# Установка кодировки для Windows (MinGW)
win32-g++:QMAKE_CXXFLAGS += -finput-charset=UTF-8 -fexec-charset=UTF-8
win32-msvc:QMAKE_CXXFLAGS += /utf-8

