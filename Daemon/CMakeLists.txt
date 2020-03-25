project( artmoo-daemon )

add_executable( ${PROJECT_NAME} WIN32 MACOSX_BUNDLE
	main.cpp

	../ServerCore/mooapplication.cpp
	../ServerCore/mooapplication.h
	)

include_directories(
	../ServerCore
	../libtelnet
	)

target_compile_definitions( ${PROJECT_NAME} PRIVATE MOO_VERSION=${MOO_VERSION} )

target_link_libraries( ${PROJECT_NAME}
	ServerCore
	SMTPEmail
	telnet
	Editor
	)

install( TARGETS ${PROJECT_NAME}
	RUNTIME DESTINATION ${PATH_APP}
	BUNDLE DESTINATION ${PATH_APP}
)

#-------------------------------------
# Qt5

find_package( Qt5 COMPONENTS Gui Network WebSockets REQUIRED )

target_link_libraries( ${PROJECT_NAME}
	Qt5::Core
	Qt5::Network
	Qt5::WebSockets
	)

include( ../qtdeploy.cmake )

#-------------------------------------
# Lua 5.1

find_package( Lua51 )

if( LUA51_FOUND )
	target_include_directories( ${PROJECT_NAME} BEFORE PRIVATE ${LUA_INCLUDE_DIR} )

	target_link_libraries( ${PROJECT_NAME} ${LUA_LIBRARIES} )
endif()

