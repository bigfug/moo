cmake_minimum_required( VERSION 2.8 )

set( MOO_DEBUG_TASKS	FALSE CACHE BOOL "Print lots of task debug" )

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
	lua_osc.cpp
	osc.cpp
	odb_file.cpp
	odb.cpp
	odb_sql.cpp
	lua_json.cpp
	lua_serialport.cpp
	lua_smtp.cpp

	inputsink/inputsinkeditor.cpp
	inputsink/inputsinkedittext.cpp
	inputsink/inputsinkprogram.cpp
	inputsink/inputsinkset.cpp
	inputsink/inputsinkread.cpp

	listeners/listenerservertcp.cpp
	listeners/listenerserverwebsocket.cpp

	listeners/listenersockettelnet.cpp

	listeners/listenersockettcp.cpp
	listeners/listenersocketwebsocket.cpp
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
	lua_utilities.h
	mooexception.h
	lua_listener.h
	lua_prop.h
	lua_text.cpp
	lua_text.h
	func.h
	connectionmanager.h
	taskentry.h
	lua_osc.h
	osc.h
	odb_file.h
	odb.h
	odb_sql.h
	lua_json.h
	lua_serialport.h
	lua_smtp.h

	inputsink/inputsinkread.h
	inputsink/inputsink.h
	inputsink/inputsinkprogram.h
	inputsink/inputsinkset.h
	inputsink/inputsinkeditor.h
	inputsink/inputsinkedittext.h

	listeners/listenerserver.h
	listeners/listenersocket.h
	listeners/listenersockettelnet.h
	listeners/listenerservertcp.h
	listeners/listenersockettcp.h
	listeners/listenerserverwebsocket.h
	listeners/listenersocketwebsocket.h

	changeset/changeset.cpp
	changeset/changeset.h
	changeset/change.h
	changeset/objectaliasadd.h
	changeset/objectaliasdelete.h
	changeset/objectcreate.h
	changeset/objectrecycle.h
	changeset/objectsetname.h
	changeset/objectsetowner.h
	changeset/objectsetread.h
	changeset/objectsetwrite.h
	changeset/objectsetlocation.h
	changeset/objectsetmodule.h
	changeset/objectsetproperty.h
	changeset/objectsetparent.h
	changeset/objectsetplayer.h
	changeset/objectsetprogrammer.h
	changeset/objectsetwizard.h
	changeset/objectsetfertile.h
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
	changeset/connectionclose.h
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

if( MOO_DEBUG_TASKS )
	target_compile_definitions( ${PROJECT_NAME} PRIVATE MOO_DEBUG_TASKS )
endif()

add_dependencies( ${PROJECT_NAME} SMTPEmail telnet lua Editor )

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
	../libs/libtelnet
	../libs/SMTPEmail/src
	${CMAKE_SOURCE_DIR}/libs/lua
	${CMAKE_BINARY_DIR}/libs
)
