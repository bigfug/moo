
project( Editor )

add_library( ${PROJECT_NAME} STATIC
	editor.cpp
	editor.h
)

find_package(Qt6 COMPONENTS Core Core5Compat)
if (NOT Qt6_FOUND)
	find_package(Qt5 5.15 REQUIRED COMPONENTS Core)
endif()

target_link_libraries( ${PROJECT_NAME}
	Qt::Core
	Qt::Core5Compat
)

