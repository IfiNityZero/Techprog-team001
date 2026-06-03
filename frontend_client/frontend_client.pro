QT += core gui network widgets

CONFIG += c++11

TARGET = TaMP_Client
TEMPLATE = app

SOURCES += \
    main.cpp \
    clientwindow.cpp \
    clientmanager.cpp

HEADERS += \
    clientwindow.h \
    clientmanager.h

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
