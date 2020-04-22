cmake_minimum_required( VERSION 3.1 )

project( ServerCoreTests )

include_directories( .. )

set( SOURCE_FILES
	tst_servertest.cpp
	tst_task.cpp
	tst_properties.cpp
	tst_verbs.cpp
	tst_lua_create.cpp
	tst_lua_recycle.cpp
	tst_lua_move.cpp
	tst_lua_parent.cpp
	tst_lua_prop.cpp
	tst_lua_verb.cpp
	tst_lua_task.cpp
	tst_lua_call_values.cpp
	tst_lua_prop_security.cpp
	permissionstest.cpp
    tst_lua_prop_inheritance.cpp
    tst_lua_verb_call.cpp
    tst_lua_pass.cpp
	tst_servertest.h
	permissionstest.h
	tst_task_rollback_object.cpp
	)

find_package( Qt5 REQUIRED COMPONENTS Core Test )

include_directories(
	../ServerCore
	../libs/libtelnet
	${CMAKE_SOURCE_DIR}/libs/lua
	${CMAKE_BINARY_DIR}/libs
)

link_libraries(
	ServerCore
	SMTPEmail
	telnet
	Editor
	lua
	Qt5::Core Qt5::Test
)

set( TEST_DATA_SOURCES luatestdata.h luatestdata.cpp )

add_executable( ${PROJECT_NAME} ${SOURCE_FILES} ${TEST_DATA_SOURCES} )

add_executable( test_object tst_object.cpp tst_object.h ${TEST_DATA_SOURCES} )

add_executable( eval_moo eval_moo.cpp eval_moo.h ${TEST_DATA_SOURCES} )

add_test( ServerCore ${PROJECT_NAME} )
add_test( test_object test_object )
add_test( eval_moo eval_moo )

