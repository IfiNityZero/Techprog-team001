QT += testlib sql network
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += tst_funcforserver_test.cpp \
    ../backend_server/functionsforserver.cpp \
    ../backend_server/mytcpserver.cpp

HEADERS += \
    ../backend_server/functionsforserver.h \
    ../backend_server/mytcpserver.h
