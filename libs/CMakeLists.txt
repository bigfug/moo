
find_package( Qt5Network REQUIRED )

add_library( SMTPEmail STATIC
	SMTPEmail/src/emailaddress.cpp
	SMTPEmail/src/mimeattachment.cpp
	SMTPEmail/src/mimefile.cpp
	SMTPEmail/src/mimehtml.cpp
	SMTPEmail/src/mimeinlinefile.cpp
	SMTPEmail/src/mimemessage.cpp
	SMTPEmail/src/mimepart.cpp
	SMTPEmail/src/mimetext.cpp
	SMTPEmail/src/smtpclient.cpp
	SMTPEmail/src/quotedprintable.cpp
	SMTPEmail/src/mimemultipart.cpp
	SMTPEmail/src/mimecontentformatter.cpp
	SMTPEmail/src/emailaddress.h
	SMTPEmail/src/mimeattachment.h
	SMTPEmail/src/mimefile.h
	SMTPEmail/src/mimehtml.h
	SMTPEmail/src/mimeinlinefile.h
	SMTPEmail/src/mimemessage.h
	SMTPEmail/src/mimepart.h
	SMTPEmail/src/mimetext.h
	SMTPEmail/src/smtpclient.h
	SMTPEmail/src/SmtpMime
	SMTPEmail/src/quotedprintable.h
	SMTPEmail/src/mimemultipart.h
	SMTPEmail/src/mimecontentformatter.h
	SMTPEmail/src/smtpexports.h
)

target_compile_definitions( SMTPEmail PRIVATE SMTP_BUILD )

target_link_libraries( SMTPEmail Qt5::Network )

add_library( telnet STATIC
	libtelnet/libtelnet.c
	libtelnet/libtelnet.h
)

if( MSVC )
	set( LUA_DIRSEP "\\\\" )
else()
	set( LUA_DIRSEP "/" )
endif()

set( LUA_PATH "LUA_PATH" CACHE STRING "Environment variable to use as package.path." )
set( LUA_CPATH "LUA_CPATH" CACHE STRING "Environment variable to use as package.cpath." )
set( LUA_IDSIZE 60 CACHE STRING "gives the maximum size for the description of the source." )

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/lua/src/luaconf.h.in ${CMAKE_CURRENT_BINARY_DIR}/luaconf.h )

SET_SOURCE_FILES_PROPERTIES(
	lua/src/lapi.c lua/src/lcode.c lua/src/ldebug.c lua/src/ldo.c lua/src/ldump.c lua/src/lfunc.c lua/src/lgc.c lua/src/llex.c
	lua/src/lmem.c lua/src/lobject.c lua/src/lopcodes.c lua/src/lparser.c lua/src/lstate.c lua/src/lstring.c
	lua/src/ltable.c lua/src/ltm.c lua/src/lundump.c lua/src/lvm.c lua/src/lzio.c
	lua/src/lauxlib.c lua/src/lbaselib.c lua/src/ldblib.c lua/src/liolib.c lua/src/lmathlib.c lua/src/loslib.c
	lua/src/ltablib.c lua/src/lstrlib.c lua/src/loadlib.c lua/src/linit.c
	PROPERTIES LANGUAGE CXX
	)

add_library( lua STATIC
	lua/src/lapi.c lua/src/lcode.c lua/src/ldebug.c lua/src/ldo.c lua/src/ldump.c lua/src/lfunc.c lua/src/lgc.c lua/src/llex.c
	lua/src/lmem.c lua/src/lobject.c lua/src/lopcodes.c lua/src/lparser.c lua/src/lstate.c lua/src/lstring.c
	lua/src/ltable.c lua/src/ltm.c lua/src/lundump.c lua/src/lvm.c lua/src/lzio.c
	lua/src/lauxlib.c lua/src/lbaselib.c lua/src/ldblib.c lua/src/liolib.c lua/src/lmathlib.c lua/src/loslib.c
	lua/src/ltablib.c lua/src/lstrlib.c lua/src/loadlib.c lua/src/linit.c
	${CMAKE_CURRENT_BINARY_DIR}/luaconf.h
	)

target_include_directories( lua PRIVATE ${CMAKE_CURRENT_BINARY_DIR} )

#if( MSVC )
#	target_compile_definitions( telnet PRIVATE __STDC__=1 )
#endif()
