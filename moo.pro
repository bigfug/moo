TEMPLATE = subdirs

SUBDIRS += \
	ServerCore \
	ServerCoreTests \
	Server \
	Editor \
	Daemon \
	libs

ServerCoreTests.depends += ServerCore
Server.depends += ServerCore
ServerCore.depends += Editor libs
Daemon.depends += ServerCore

linux {
	SUBDIRS += EditorTest

	EditorTest.depends += Editor
}

