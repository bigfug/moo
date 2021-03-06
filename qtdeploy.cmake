
find_package( Qt5
	COMPONENTS Core Gui Network Widgets WebSockets SerialPort LinguistTools
	QUIET )

target_link_libraries( ${PROJECT_NAME} Qt5::Core Qt5::Gui Qt5::Network Qt5::Widgets Qt5::WebSockets Qt5::SerialPort )

# Retrieve the absolute path to qmake and then use that path to find
# the binaries
get_target_property(_qmake_executable Qt5::qmake IMPORTED_LOCATION)
get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)

if( WIN32 AND CMAKE_BUILD_TYPE STREQUAL Release )
	find_program( WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}" )

	get_filename_component( ABS_BINARY_DIR "${CMAKE_INSTALL_PREFIX}" REALPATH BASE_DIR "${CMAKE_BINARY_DIR}")

	# Run windeployqt immediately after build
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
	  COMMAND "${WINDEPLOYQT_EXECUTABLE}"
		--verbose 2
		--no-compiler-runtime
		--serialport --websockets --network
		--dir "${ABS_BINARY_DIR}/${PATH_APP}"
		--libdir "${ABS_BINARY_DIR}/${PATH_APP}"
		--plugindir "${ABS_BINARY_DIR}/${PATH_APP}"
		\"$<TARGET_FILE:${PROJECT_NAME}>\"
	)

	file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_$<CONFIG>_path"
		CONTENT "$<TARGET_FILE:${PROJECT_NAME}>"
	)

	# Before installation, run a series of commands that copy each of the Qt
	# runtime files to the appropriate directory for installation
	install( CODE
		"
		file(READ \"${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_Release_path\" _file)
		execute_process(
			COMMAND \"${WINDEPLOYQT_EXECUTABLE}\"
					--dry-run
					--no-compiler-runtime
					--list mapping
					--serialport --websockets --network
					--dir \"${ABS_BINARY_DIR}/${PATH_APP}\"
					--libdir \"${ABS_BINARY_DIR}/${PATH_APP}\"
					--plugindir \"${ABS_BINARY_DIR}/${PATH_APP}\"
					\${_file}
			OUTPUT_VARIABLE _output
			OUTPUT_STRIP_TRAILING_WHITESPACE
		)

		separate_arguments(_files WINDOWS_COMMAND \${_output})

		while(_files)
			list(GET _files 0 _src)
			list(GET _files 1 _dest)
			message( \${_src} )
			execute_process(
				COMMAND \"${CMAKE_COMMAND}\" -E
				copy \${_src} \"\${CMAKE_INSTALL_PREFIX}/${PATH_APP}/\${_dest}\"
			)
			separate_arguments(_files WINDOWS_COMMAND \${_output})
			while(_files)
					list(GET _files 0 _src)
					list(GET _files 1 _dest)
					execute_process(
							COMMAND \"${CMAKE_COMMAND}\" -E
							copy \${_src} \"\${CMAKE_INSTALL_PREFIX}/${PATH_APP}/\${_dest}\"
					)
					list(REMOVE_AT _files 0 1)
			endwhile()
		endwhile()
		"
	)

	# windeployqt doesn't work correctly with the system runtime libraries,
	# so we fall back to one of CMake's own modules for copying them over
	set(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)
	include(InstallRequiredSystemLibraries)
	foreach(lib ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS})
		get_filename_component(filename "${lib}" NAME)
		add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
			COMMAND "${CMAKE_COMMAND}" -E
				copy_if_different "${lib}" \"$<TARGET_FILE_DIR:${PROJECT_NAME}>\"
		)
	endforeach()

endif()

if( APPLE AND CMAKE_BUILD_TYPE STREQUAL Release )

	find_program( MACDEPLOYQT_EXECUTABLE macdeployqt HINTS "${_qt_bin_dir}" )

	set_target_properties( ${PROJECT_NAME} PROPERTIES INSTALL_RPATH “@loader_path/../Frameworks” )

	add_custom_command( TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND "${MACDEPLOYQT_EXECUTABLE}"
			"$<TARGET_FILE_DIR:${PROJECT_NAME}>/../.."
			-always-overwrite
		COMMENT "Running macdeployqt..."
	)

endif()
