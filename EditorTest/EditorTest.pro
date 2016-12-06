QT += core
QT -= gui

CONFIG += c++11

TARGET = EditorTest
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
	editorapplication.cpp \
	cursesreader.cpp

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

HEADERS += \
	editorapplication.h \
	cursesreader.h

macx {
	LIBS += -lcurses
}
