project( Daemon )

add_executable( ${PROJECT_NAME} WIN32 MACOSX_BUNDLE main.cpp )

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

install( TARGETS ${PROJECT_NAME} DESTINATION . )


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

find_package( Lua51 REQUIRED )

if( LUA51_FOUND )
        include_directories( ${LUA_INCLUDE_DIR} )

        target_link_libraries( ${PROJECT_NAME} ${LUA_LIBRARIES} )
endif()

