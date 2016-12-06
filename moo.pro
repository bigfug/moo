TEMPLATE = subdirs

SUBDIRS += \
	ServerCore \
	ServerCoreTests \
	Server \
	Editor \
	EditorTest

EditorTest.depends += Editor
