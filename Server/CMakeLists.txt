
if( UNIX AND NOT APPLE )
	project( artmoo-server )
else()
	project( ArtMOO )
endif()

set( SOURCE_FILES
	main.cpp

	mainwindow.cpp
	mainwindow.h
	mainwindow.ui

	objectselector.cpp
	objectselector.h
	objectselector.ui

	mooitemmodel.cpp
	mooitemmodel.h

	codeeditor.cpp
	codeeditor.h
	linenumberarea.cpp
	linenumberarea.h
	syntaxhighlighterlua.cpp
	syntaxhighlighterlua.h

	Info.plist

	../ServerCore/mooapplication.cpp
	../ServerCore/mooapplication.h
)

set( QRC_FILES
	Server.qrc
)

add_executable( ${PROJECT_NAME} WIN32 MACOSX_BUNDLE ${SOURCE_FILES} ${QRC_FILES} )

target_compile_definitions( ${PROJECT_NAME} PRIVATE MOO_VERSION=${MOO_VERSION} )

include_directories(
	../ServerCore
	../libs/libtelnet
	${CMAKE_SOURCE_DIR}/libs/lua
	${CMAKE_BINARY_DIR}/libs
)

target_link_libraries( ${PROJECT_NAME}
	ServerCore
	SMTPEmail
	telnet
	Editor
	lua
)

if( APPLE )
	set( MOO_ICON moo-icon.icns )

	set_target_properties( ${PROJECT_NAME} PROPERTIES
		MACOSX_FRAMEWORK_IDENTIFIER com.bigfug.moo.server
		MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist
		MACOSX_BUNDLE_ICON_FILE ${MOO_ICON}
	)

	set_source_files_properties( ${MOO_ICON} PROPERTIES
		MACOSX_PACKAGE_LOCATION "Resources"
		)

	target_sources( ${PROJECT_NAME} PRIVATE
		${MOO_ICON}
		)

endif( APPLE )


install( TARGETS ${PROJECT_NAME}
	RUNTIME DESTINATION ${PATH_APP}
	BUNDLE DESTINATION ${PATH_APP}
)

#-------------------------------------
# Qt5

include( ../qtdeploy.cmake )
