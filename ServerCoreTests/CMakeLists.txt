cmake_minimum_required( VERSION 3.1 )

project( ServerCoreTests )

include_directories( .. )

set( SOURCE_FILES
	tst_servertest.cpp
	tst_servertest.h

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
	tst_lua_prop_security.cpp
    tst_lua_prop_inheritance.cpp
    tst_lua_verb_call.cpp
    tst_lua_pass.cpp
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
add_executable( permissionstest permissionstest.cpp permissionstest.h ${TEST_DATA_SOURCES} )
add_executable( task_rollback task_rollback.cpp task_rollback.h ${TEST_DATA_SOURCES} )
add_executable( tst_lua_call_values tst_lua_call_values.cpp tst_lua_call_values.h ${TEST_DATA_SOURCES} )

add_test( ServerCore ${PROJECT_NAME} )
add_test( test_object test_object )
add_test( eval_moo eval_moo )
add_test( permissionstest permissionstest )
add_test( task_rollback task_rollback )
add_test( tst_lua_call_values tst_lua_call_values )


