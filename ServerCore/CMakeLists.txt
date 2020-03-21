cmake_minimum_required( VERSION 2.8 )

project( ServerCore )

set( SOURCE_FILES
	verb.cpp
	task.cpp
	property.cpp
	objectmanager.cpp
	objectlogic.cpp
	object.cpp
	mooapp.cpp
	lua_verb.cpp
	lua_task.cpp
	lua_object.cpp
	lua_moo.cpp
	lua_connection.cpp
	connection.cpp
	lua_utilities.cpp
	mooexception.cpp
	lua_listener.cpp
	lua_prop.cpp
	func.cpp
	connectionmanager.cpp
	taskentry.cpp
	inputsinkprogram.cpp
	inputsinkset.cpp
	lua_osc.cpp
	osc.cpp
	inputsinkeditor.cpp
	inputsinkedittext.cpp
	odb_file.cpp
	odb.cpp
	odb_sql.cpp
	lua_json.cpp
	serialport.cpp
	lua_serialport.cpp
	listenertelnet.cpp
	listenertelnetsocket.cpp
	listenerwebsocket.cpp
	listenerserver.cpp
	listenersocket.cpp
	listenerwebsocketsocket.cpp
	lua_smtp.cpp
	inputsinkread.cpp
)

set( HEADER_FILES
	verb.h
	task.h
	property.h
	objectmanager.h
	objectlogic.h
	object.h
	mooglobal.h
	mooapp.h
	lua_verb.h
	lua_task.h
	lua_object.h
	lua_moo.h
	lua_connection.h
	connection.h
	stdafx.h
	lua_utilities.h
	mooexception.h
	lua_listener.h
	lua_prop.h
	func.h
	connectionmanager.h
	taskentry.h
	inputsink.h
	inputsinkprogram.h
	inputsinkset.h
	lua_osc.h
	osc.h
	inputsinkeditor.h
	inputsinkedittext.h
	odb_file.h
	odb.h
	odb_sql.h
	lua_json.h
	serialport.h
	lua_serialport.h
	listenertelnet.h
	listenertelnetsocket.h
	listenerwebsocket.h
	listenerserver.h
	listenersocket.h
	listenerwebsocketsocket.h
	lua_smtp.h
	inputsinkread.h
	changeset/changeset.cpp
	changeset/changeset.h
	changeset/change.h
	changeset/objectaliasadd.h
	changeset/objectaliasdelete.h
	changeset/objectcreate.h
	changeset/objectsetname.h
	changeset/objectsetowner.h
	changeset/objectsetproperty.h
	changeset/objectverbadd.h
	changeset/objectverbdelete.h
	changeset/objectpropadd.h
	changeset/objectpropclear.h
	changeset/objectpropdelete.h
	changeset/propertysetowner.h
	changeset/propertysetread.h
	changeset/propertysetwrite.h
	changeset/propertysetchange.h
	changeset/connectionnotify.h
	changeset/connectionsetcookie.h
	changeset/connectionclearcookie.h
	changeset/verbsetowner.h
	changeset/verbsetread.h
	changeset/verbsetwrite.h
	changeset/verbsetexecute.h
	changeset/verbsetscript.h
	changeset/verbsetdirectobject.h
	changeset/verbsetindirectobject.h
	changeset/verbsetproposition.h
	changeset/verbaliasadd.h
	changeset/verbaliasdelete.h
)

add_library( ${PROJECT_NAME} STATIC ${SOURCE_FILES} ${HEADER_FILES} )

add_dependencies( ${PROJECT_NAME} SMTPEmail telnet Editor )

find_package( Qt5 COMPONENTS Core Network Sql SerialPort Xml WebSockets REQUIRED )

target_link_libraries( ${PROJECT_NAME}
	Qt5::Core
	Qt5::Network
	Qt5::Sql
	Qt5::SerialPort
	Qt5::Xml
	Qt5::WebSockets
)

include_directories(
	../Editor
	../libtelnet
	../libs/SMTPEmail/src
)

find_package( Lua51 )

if( LUA51_FOUND )
	target_include_directories( ${PROJECT_NAME} BEFORE PRIVATE ${LUA_INCLUDE_DIR} )

	target_link_libraries( ${PROJECT_NAME} ${LUA_LIBRARIES} )
endif()

