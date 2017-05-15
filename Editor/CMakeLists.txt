
project( Editor )

add_library( ${PROJECT_NAME} STATIC
	Editor.cpp
	Editor.h
)

find_package( Qt5 COMPONENTS Core REQUIRED )

target_link_libraries( ${PROJECT_NAME}
	Qt5::Core
)

