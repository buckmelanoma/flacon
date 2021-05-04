
macro(install_qt5_plugin)
	cmake_parse_arguments(_ARGS "" "DESTINATION" "" ${ARGN})
	set(_plugin ${_ARGS_UNPARSED_ARGUMENTS})
	set(_dest ${_ARGS_DESTINATION} )

	get_target_property(_qt_plugin_path "${_plugin}" LOCATION)
	if(EXISTS "${_qt_plugin_path}")
		get_filename_component(_qt_plugin_file "${_qt_plugin_path}" NAME)
		get_filename_component(_qt_plugin_type "${_qt_plugin_path}" PATH)
		get_filename_component(_qt_plugin_type "${_qt_plugin_type}" NAME)
		install(FILES "${_qt_plugin_path}" DESTINATION "${_dest}/${_qt_plugin_type}")
	else()
		message(FATAL_ERROR "QT plugin ${_plugin} not found")
	endif()
endmacro()

macro(install_qt5_plugins)
	cmake_parse_arguments(_ARGS "" "DESTINATION" "" ${ARGN})
	set(_plugins ${_ARGS_UNPARSED_ARGUMENTS})
	set(_dest ${_ARGS_DESTINATION} )

	foreach ( _plugin ${_plugins})
		install_qt5_plugin(${_plugin} DESTINATION ${_dest})

	endforeach()
endmacro()


function(install_third_party_binaries)
    cmake_parse_arguments(ARGS "" "DESTINATION" "PATHS" ${ARGN})
    set(names ${ARGS_UNPARSED_ARGUMENTS})
    file(TO_CMAKE_PATH ${ARGS_PATHS} paths)
    set(dest  ${ARGS_DESTINATION})

    foreach(name ${names})
        find_program(${name}_PATH "${name}" REQUIRED PATHS ${paths} NO_DEFAULT_PATH)
        if (${name}_PATH MATCHES ".*NOTFOUND" )
            message(FATAL_ERROR "Not found ${name} binary.")
        endif()

        get_filename_component(bin ${${name}_PATH} REALPATH)

        install(PROGRAMS ${bin} DESTINATION ${dest})
    endforeach()
endfunction()

function(fixup_qt5_bundle _BUNDLE_PATH _DIRS)

	# **************************************
	# Pass variables to INSTALL code
	install(CODE "set(_BUNDLE_PATH \"${_BUNDLE_PATH}\")")
	install(CODE "set(_DIRS \"${_DIRS}\")")

	# **************************************
	# INSTALL Qt config file
	install(CODE [[
		message("###################################")
		message("## Creating Qt config file")

		set(QT_CONF_PATH "${_BUNDLE_PATH}/Contents/Resources/qt.conf")
		set(QT_CONF_CONTENT "[Paths]\nPlugins = PlugIns\n")

		file(MAKE_DIRECTORY "${_BUNDLE_PATH}/Contents/Resources/")
		file(WRITE "${QT_CONF_PATH}" "${QT_CONF_CONTENT}")

	]] COMPONENT Runtime)

	# **************************************
	# FixUp bundle
	install(CODE [[
		message("###################################")
		message("## Fixuping bundle")

		file(GLOB_RECURSE plugins
			LIST_DIRECTORIES false
			${_BUNDLE_PATH}/Contents/PlugIns/*
		)

		include(BundleUtilities)
		fixup_bundle("${_BUNDLE_PATH}" "${plugins}" "${_DIRS}")

	]] COMPONENT Runtime )


endfunction()


function(sign_bundle _BUNDLE_PATH _CERT_IDENTITY)

	# **************************************
	# Pass variables to INSTALL code
	install(CODE "set(_BUNDLE_PATH \"${_BUNDLE_PATH}\")")
	install(CODE "set(_CERT_IDENTITY \"${_CERT_IDENTITY}\")")


	install(CODE [[
		message("###################################")
		message("## Signing the bundle")

		execute_process(COMMAND
			codesign
		 	--force
		 	--deep
		 	--verify
		 	--sign "${_CERT_IDENTITY}"
		 	"${_BUNDLE_PATH}"
			)

		message("###################################")
		message("## Verifying the signature")
		execute_process(COMMAND
			codesign
			-v
			--strict
			--deep
			--verbose=1
			"${_BUNDLE_PATH}"
		)

	]])
endfunction()
