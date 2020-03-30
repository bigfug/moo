
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

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/lua/luaconf.h ${CMAKE_CURRENT_BINARY_DIR}/luaconf.h )

SET_SOURCE_FILES_PROPERTIES(
	lua/lapi.c lua/lcode.c lua/lctype.c lua/ldebug.c lua/ldo.c lua/ldump.c lua/lfunc.c lua/lgc.c lua/llex.c
	lua/lmem.c lua/lobject.c lua/lopcodes.c lua/lparser.c lua/lstate.c lua/lstring.c lua/ltable.c
	lua/ltm.c lua/lundump.c lua/lvm.c lua/lzio.c lua/ltests.c
	lua/lauxlib.c
	lua/lbaselib.c lua/ldblib.c lua/liolib.c lua/lmathlib.c lua/loslib.c lua/ltablib.c lua/lstrlib.c
	lua/lutf8lib.c lua/lbitlib.c lua/loadlib.c lua/lcorolib.c lua/linit.c
	PROPERTIES LANGUAGE CXX
	)

add_library( lua STATIC
	lua/lapi.c lua/lcode.c lua/lctype.c lua/ldebug.c lua/ldo.c lua/ldump.c lua/lfunc.c lua/lgc.c lua/llex.c
	lua/lmem.c lua/lobject.c lua/lopcodes.c lua/lparser.c lua/lstate.c lua/lstring.c lua/ltable.c
	lua/ltm.c lua/lundump.c lua/lvm.c lua/lzio.c lua/ltests.c
	lua/lauxlib.c
	lua/lbaselib.c lua/ldblib.c lua/liolib.c lua/lmathlib.c lua/loslib.c lua/ltablib.c lua/lstrlib.c
	lua/lutf8lib.c lua/lbitlib.c lua/loadlib.c lua/lcorolib.c lua/linit.c
	${CMAKE_CURRENT_BINARY_DIR}/luaconf.h
	)

target_include_directories( lua PRIVATE ${CMAKE_CURRENT_BINARY_DIR} )

#if( MSVC )
#	target_compile_definitions( telnet PRIVATE __STDC__=1 )
#endif()
