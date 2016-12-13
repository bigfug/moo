QT += core network
QT -= gui

CONFIG += c++11

TARGET = Daemon
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp

#----------------------------------------------------------------------------

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../ServerCore/release/ -lServerCore
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../ServerCore/debug/ -lServerCore
else:unix: LIBS += -L$$OUT_PWD/../ServerCore/ -lServerCore

INCLUDEPATH += $$PWD/../ServerCore
DEPENDPATH += $$PWD/../ServerCore

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../ServerCore/release/libServerCore.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../ServerCore/debug/libServerCore.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../ServerCore/release/ServerCore.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../ServerCore/debug/ServerCore.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../ServerCore/libServerCore.a

#----------------------------------------------------------------------------
# LUA

windows {
	INCLUDEPATH += $$(LIBS)/Lua-5.1.4/include

	LIBS += -L$$(LIBS)/Lua-5.1.4 -llua5.1
}

macx {
	INCLUDEPATH += /usr/local/opt/lua51/include/lua-5.1

	LIBS += -L/usr/local/opt/lua51/lib -llua5.1
}

linux:!macx:exists( /usr/include/lua5.1 ) {
	INCLUDEPATH += /usr/include/lua5.1

	LIBS += -llua5.1
}

#----------------------------------------------------------------------------
# libtelnet

INCLUDEPATH += ../libtelnet
