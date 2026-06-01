QT += core network sql
QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle

TARGET = echoServer
TEMPLATE = app

SOURCES += \
    main.cpp \
    mytcpserver.cpp \
    functionsforserver.cpp \
    database.cpp

HEADERS += \
    mytcpserver.h \
    functionsforserver.h \
    database.h
