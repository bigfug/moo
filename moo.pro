TEMPLATE = subdirs

SUBDIRS += \
	ServerCore \
	ServerCoreTests \
	Server \
	Editor

ServerCoreTests.depends += ServerCore
Server.depends += ServerCore
ServerCore.depends += Editor

linux {
	SUBDIRS += EditorTest

	EditorTest.depends += Editor
}

