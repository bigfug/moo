QT += network widgets

HEADERS += \
	mainwindow.h

SOURCES += \
	mainwindow.cpp \
	main.cpp

FORMS += \
	mainwindow.ui

RC_FILE = Server.rc

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../ServerCore/release/ -lServerCore
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../ServerCore/debug/ -lServerCore
else:symbian: LIBS += -lServerCore
else:unix: LIBS += -L$$OUT_PWD/../ServerCore/ -lServerCore

INCLUDEPATH += $$PWD/../ServerCore
DEPENDPATH += $$PWD/../ServerCore

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../ServerCore/release/libServerCore.a
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../ServerCore/debug/libServerCore.a
else:unix:!symbian: PRE_TARGETDEPS += $$OUT_PWD/../ServerCore/libServerCore.a

#----------------------------------------------------------------------------
# LUA

#win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../lua514-build-desktop/release/ -llua514
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../lua514-build-desktop/debug/ -llua514
#else:symbian: LIBS += -llua514
#else:unix: LIBS += -L$$PWD/../../lua514-build-desktop/ -llua514

#INCLUDEPATH += $$PWD/../../lua-5.1.4/src
#DEPENDPATH += $$PWD/../../lua-5.1.4/src
#INCLUDEPATH += $$PWD/../../lua-5.1.4/etc
#DEPENDPATH += $$PWD/../../lua-5.1.4/etc

#win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../lua514-build-desktop/release/liblua514.a
#else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../lua514-build-desktop/debug/liblua514.a
#else:unix:!symbian: PRE_TARGETDEPS += $$PWD/../lua514-build-desktop/liblua514.a

windows {
	LIBS += -L$$PWD/../lua/ -llua5.1

	INCLUDEPATH += $$PWD/../lua
	DEPENDPATH += $$PWD/../lua
}

macx {
	INCLUDEPATH += /usr/local/opt/lua51/include/lua-5.1

	LIBS += -L/usr/local/opt/lua51/lib -llua5.1
}

#----------------------------------------------------------------------------

#INCLUDEPATH += $$PWD/../../bullet-2.80/src
#DEPENDPATH += $$PWD/../../bullet-2.80/src
#LIBS += -L$$PWD/../../bullet-2.80/lib/ -lBulletWorldImporter -lBulletFileLoader -lBulletDynamics -lBulletCollision -lLinearMath

OTHER_FILES += \
	Server.rc \
	moo-icon.ico

RESOURCES += \
	Server.qrc
