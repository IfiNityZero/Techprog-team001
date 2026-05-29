QT += testlib sql network
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app

SOURCES +=  tst_funcforserver_test.cpp \
    ../EchoServer/functionsforserver.cpp \
    ../EchoServer/mytcpserver.cpp

HEADERS += \
    ../EchoServer/dataBase.h \
    ../EchoServer/functionsforserver.h \
    ../EchoServer/mytcpserver.h
