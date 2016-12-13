#-------------------------------------------------
#
# Project created by QtCreator 2011-10-18T13:23:37
#
#-------------------------------------------------

QT       -= gui
QT       += network sql

TARGET = ServerCore
TEMPLATE = lib
CONFIG += staticlib create_prl c++11

windows: DEFINES += LUA_BUILD_AS_DLL

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
	inputsinkprogram.cpp \
	inputsinkset.cpp \
	lua_osc.cpp \
	osc.cpp \
	inputsinkeditor.cpp \
	inputsinkedittext.cpp \
	odb_file.cpp \
	odb.cpp \
	odb_sql.cpp

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
	inputsink.h \
	inputsinkprogram.h \
	inputsinkset.h \
	lua_osc.h \
	osc.h \
	inputsinkeditor.h \
	inputsinkedittext.h \
	odb_file.h \
	odb.h \
	odb_sql.h

unix:!symbian {
	maemo5 {
		target.path = /opt/usr/lib
	} else {
		target.path = /usr/lib
	}
	INSTALLS += target
}

macx {
	INCLUDEPATH += /usr/local/opt/lua51/include/lua-5.1

	LIBS += -L/usr/local/opt/lua51/lib -llua5.1
}

windows {
	INCLUDEPATH += $$(LIBS)/Lua-5.1.4/include

	LIBS += -L$$(LIBS)/Lua-5.1.4 -llua5.1
}

DISTFILES += \
	../LICENSE \
	../README.md

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../Editor/release/ -lEditor
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../Editor/debug/ -lEditor
else:unix: LIBS += -L$$OUT_PWD/../Editor/ -lEditor

INCLUDEPATH += $$PWD/../Editor
DEPENDPATH += $$PWD/../Editor

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Editor/release/libEditor.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Editor/debug/libEditor.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Editor/release/Editor.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Editor/debug/Editor.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../Editor/libEditor.a

#----------------------------------------------------------------------------
# libtelnet

SOURCES += ../libtelnet/libtelnet.c
HEADERS += ../libtelnet/libtelnet.h
INCLUDEPATH += ../libtelnet

