
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
	../libtelnet/libtelnet.c
	../libtelnet/libtelnet.h
)

#if( MSVC )
#	target_compile_definitions( telnet PRIVATE __STDC__=1 )
#endif()
