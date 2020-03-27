cmake_minimum_required( VERSION 3.1 )

project( ServerCoreTests )

include_directories( .. )

set( SOURCE_FILES
	tst_servertest.cpp
	tst_task.cpp
	tst_properties.cpp
	tst_verbs.cpp
	tst_object.cpp
	tst_lua_create.cpp
	tst_lua_recycle.cpp
	tst_lua_move.cpp
	tst_lua_parent.cpp
	tst_lua_prop.cpp
	tst_lua_verb.cpp
	tst_lua_task.cpp
	tst_lua_call_values.cpp
	luatestdata.cpp
	tst_lua_prop_security.cpp
	permissionstest.cpp
    tst_lua_prop_inheritance.cpp
    tst_lua_verb_call.cpp
    tst_lua_pass.cpp
	tst_servertest.h
	stdafx.h
	luatestdata.h
	permissionstest.h
	tst_task_rollback_object.cpp
	)

add_executable( ${PROJECT_NAME} ${SOURCE_FILES} )

find_package( Qt5 REQUIRED COMPONENTS Core Test )

add_test( ServerCore ${PROJECT_NAME} )

include_directories(
	../ServerCore
	../libs/libtelnet
	../libs/lua/src
	../libs/lua/etc
	${CMAKE_BINARY_DIR}/libs
)

target_link_libraries( ${PROJECT_NAME}
	ServerCore
	SMTPEmail
	telnet
	Editor
	lua
	Qt5::Core Qt5::Test
)
