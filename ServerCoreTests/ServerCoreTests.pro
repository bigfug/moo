#-------------------------------------------------
#
# Project created by QtCreator 2011-10-06T16:39:10
#
#-------------------------------------------------

QT       += gui testlib

TARGET = tst_servertest
CONFIG   += console
CONFIG   -= app_bundle
CONFIG   += link_prl

TEMPLATE = app

PRECOMPILED_HEADER = stdafx.h

SOURCES += tst_servertest.cpp \
	tst_task.cpp \
	tst_properties.cpp \
	tst_verbs.cpp \
	tst_object.cpp \
	tst_lua_create.cpp \
	tst_lua_recycle.cpp \
	tst_lua_move.cpp \
	tst_lua_parent.cpp \
	tst_lua_prop.cpp \
	tst_lua_verb.cpp \
	tst_lua_task.cpp \
	tst_lua_call_values.cpp \
	luatestdata.cpp \
	tst_lua_prop_security.cpp \
	permissionstest.cpp \
    tst_lua_prop_inheritance.cpp

HEADERS += tst_servertest.h \
	stdafx.h \
	luatestdata.h \
	permissionstest.h

DEFINES += SRCDIR=\\\"$$PWD/\\\"

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
