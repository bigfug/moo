cmake_minimum_required( VERSION 3.0 )

project( moo )

# Instruct CMake to run moc automatically when needed.
set( CMAKE_AUTOMOC ON )
set( CMAKE_AUTOUIC ON )
set( CMAKE_AUTORCC ON )

set( CMAKE_INCLUDE_CURRENT_DIR ON )

set( CMAKE_CXX_STANDARD 11 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_CXX_EXTENSIONS OFF )

add_subdirectory( Editor )
add_subdirectory( libs )
add_subdirectory( Server )
add_subdirectory( ServerCore )
add_subdirectory( ServerCoreTests )
add_subdirectory( Daemon )

# The dependencies for various UNIX packages

if( UNIX AND NOT APPLE )
	set( CPACK_DEBIAN_PACKAGE_DEBUG ON )
	set( CPACK_DEB_COMPONENT_INSTALL ON )
	set( CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON )
	set( CPACK_DEBIAN_PACKAGE_GENERATE_SHLIBS ON )
endif()

include( CPack )


