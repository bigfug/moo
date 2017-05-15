cmake_minimum_required( VERSION 2.8 )

project( moo )

# Instruct CMake to run moc automatically when needed.
set( CMAKE_AUTOMOC ON )
set( CMAKE_AUTOUIC ON )
set( CMAKE_AUTORCC ON )

set( CMAKE_INCLUDE_CURRENT_DIR ON )

add_subdirectory( Editor )
add_subdirectory( libs )
add_subdirectory( Server )
add_subdirectory( ServerCore )
add_subdirectory( Daemon )

include( CPack )


