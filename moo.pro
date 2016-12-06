TEMPLATE = subdirs

SUBDIRS += \
	ServerCore \
	ServerCoreTests \
	Server \
	Editor \
	EditorTest

EditorTest.depends += Editor
ServerCoreTests.depends += ServerCore
Server.depends += ServerCore
ServerCore.depends += Editor

