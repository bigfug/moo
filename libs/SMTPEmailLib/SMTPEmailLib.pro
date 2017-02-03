#-------------------------------------------------
#
# Project created by QtCreator 2011-08-11T20:59:25
#
#-------------------------------------------------

QT       += core network

TARGET = SMTPEmailLib

# Build as an application
#TEMPLATE = app

# Build as a library
TEMPLATE = lib
DEFINES += SMTP_BUILD
CONFIG += staticlib

SOURCES += \
	../SMTPEmail/src/emailaddress.cpp \
	../SMTPEmail/src/mimeattachment.cpp \
	../SMTPEmail/src/mimefile.cpp \
	../SMTPEmail/src/mimehtml.cpp \
	../SMTPEmail/src/mimeinlinefile.cpp \
	../SMTPEmail/src/mimemessage.cpp \
	../SMTPEmail/src/mimepart.cpp \
	../SMTPEmail/src/mimetext.cpp \
	../SMTPEmail/src/smtpclient.cpp \
	../SMTPEmail/src/quotedprintable.cpp \
	../SMTPEmail/src/mimemultipart.cpp \
	../SMTPEmail/src/mimecontentformatter.cpp \

HEADERS  += \
	../SMTPEmail/src/emailaddress.h \
	../SMTPEmail/src/mimeattachment.h \
	../SMTPEmail/src/mimefile.h \
	../SMTPEmail/src/mimehtml.h \
	../SMTPEmail/src/mimeinlinefile.h \
	../SMTPEmail/src/mimemessage.h \
	../SMTPEmail/src/mimepart.h \
	../SMTPEmail/src/mimetext.h \
	../SMTPEmail/src/smtpclient.h \
	../SMTPEmail/src/SmtpMime \
	../SMTPEmail/src/quotedprintable.h \
	../SMTPEmail/src/mimemultipart.h \
	../SMTPEmail/src/mimecontentformatter.h \
	../SMTPEmail/src/smtpexports.h

OTHER_FILES += \
	../SMTPEmail/LICENSE \
	../SMTPEmail/README.md

FORMS +=
