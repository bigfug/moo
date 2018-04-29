project( Server )

set( SOURCE_FILES
	main.cpp
	mainwindow.cpp
)

set( HEADER_FILES
	mainwindow.h
	mainwindow.ui
)

set( QRC_FILES
	Server.qrc
)

add_executable( ${PROJECT_NAME} WIN32 MACOSX_BUNDLE ${SOURCE_FILES} ${HEADER_FILES} ${QRC_FILES} )

include_directories(
	../ServerCore
	../libtelnet
)

target_link_libraries( ${PROJECT_NAME}
	ServerCore
	SMTPEmail
	telnet
	Editor
)

if( APPLE )
set_target_properties( ${PROJECT_NAME} PROPERTIES
	MACOSX_FRAMEWORK_ICON_FILE moo-icon.ico
	MACOSX_FRAMEWORK_IDENTIFIER com.bigfug.moo.server
)
endif( APPLE )

install( TARGETS ${PROJECT_NAME} DESTINATION . )

#-------------------------------------
# Qt5

find_package( Qt5 COMPONENTS Gui Network WebSockets Widgets REQUIRED )

target_link_libraries( ${PROJECT_NAME}
	Qt5::Core
	Qt5::Gui
	Qt5::Network
	Qt5::WebSockets
	Qt5::Widgets
)

include( ../qtdeploy.cmake )

#-------------------------------------
# Lua 5.1

find_package( Lua51 REQUIRED )

if( LUA51_FOUND )
	target_include_directories( ${PROJECT_NAME} BEFORE PRIVATE ${LUA_INCLUDE_DIR} )

        target_link_libraries( ${PROJECT_NAME} ${LUA_LIBRARIES} )
endif()

