project( artmoo-daemon )

add_executable( ${PROJECT_NAME} WIN32 MACOSX_BUNDLE
	main.cpp

	../ServerCore/mooapplication.cpp
	../ServerCore/mooapplication.h
	)

include_directories(
	../ServerCore
	../libs/libtelnet
	../libs/lua/src
	../libs/lua/etc
	${CMAKE_BINARY_DIR}/libs
	)

target_compile_definitions( ${PROJECT_NAME} PRIVATE MOO_VERSION=${MOO_VERSION} )

target_link_libraries( ${PROJECT_NAME}
	ServerCore
	SMTPEmail
	telnet
	Editor
	lua
	)

install( TARGETS ${PROJECT_NAME}
	RUNTIME DESTINATION ${PATH_APP}
	BUNDLE DESTINATION ${PATH_APP}
)

if( APPLE )
	set( MOO_ICON ../Server/moo-icon.icns )

	set_target_properties( ${PROJECT_NAME} PROPERTIES
		MACOSX_FRAMEWORK_IDENTIFIER com.bigfug.moo.daemon
#		MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist
		MACOSX_BUNDLE_ICON_FILE ${MOO_ICON}
	)

	set_source_files_properties( ${MOO_ICON} PROPERTIES
		MACOSX_PACKAGE_LOCATION "Resources"
		)

	target_sources( ${PROJECT_NAME} PRIVATE
		${MOO_ICON}
		)
endif( APPLE )

#-------------------------------------
# Qt5

include( ../qtdeploy.cmake )
