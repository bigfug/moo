#-------------------------------------------------
#
# Project created by QtCreator 2011-10-18T13:23:37
#
#-------------------------------------------------

QT       -= gui
QT       += network

TARGET = ServerCore
TEMPLATE = lib
CONFIG += staticlib create_prl

# DEFINES += LUA_BUILD_AS_DLL

PRECOMPILED_HEADER = stdafx.h

SOURCES += \
    verb.cpp \
    task.cpp \
    property.cpp \
    objectmanager.cpp \
    objectlogic.cpp \
    object.cpp \
    mooapp.cpp \
    lua_verb.cpp \
    lua_task.cpp \
    lua_object.cpp \
    lua_moo.cpp \
    lua_connection.cpp \
    listener.cpp \
    connection.cpp \
    lua_utilities.cpp \
    mooexception.cpp \
    lua_listener.cpp \
    lua_prop.cpp \
    func.cpp \
    connectionmanager.cpp \
    taskentry.cpp \
    physicsmanager.cpp \
    inputsinkprogram.cpp \
    inputsinkset.cpp

HEADERS += \
    verb.h \
    task.h \
    property.h \
    objectmanager.h \
    objectlogic.h \
    object.h \
    mooglobal.h \
    mooapp.h \
    lua_verb.h \
    lua_task.h \
    lua_object.h \
    lua_moo.h \
    lua_connection.h \
    listener.h \
    connection.h \
    stdafx.h \
    lua_utilities.h \
    mooexception.h \
    lua_listener.h \
    lua_prop.h \
    func.h \
    connectionmanager.h \
    taskentry.h \
    physicsmanager.h \
    inputsink.h \
    inputsinkprogram.h \
    inputsinkset.h

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}

macx {
    INCLUDEPATH += /opt/local/include/lua-5.1

    LIBS += -L/opt/local/lib/lua-5.1 -llua-5.1
}

DISTFILES += \
    ../LICENSE \
    ../README.md
