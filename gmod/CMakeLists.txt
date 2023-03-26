function(set_gmod_suffix_prefix_better library)
    set(prefixCL "gmcl_")
    set(prefixSV "gmsv_")
    set(suffix "")

	if(APPLE)
		if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
            set(suffix "_osx.dll")
		else()
            set(suffixX "_osx64.dll")
		endif()
	elseif(UNIX)
		if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
            set(suffix "_linux.dll")
		else()
            set(suffix "_linux64.dll")
		endif()
	elseif(WIN32)
		if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
            set(suffix "_win32.dll")
		else()
            set(suffix "_win64.dll")
		endif()
	endif()

	if(MSVC_IDE)
		SET_TARGET_PROPERTIES(${library} PROPERTIES PREFIX ${prefixSV})
	else()
		SET_TARGET_PROPERTIES(${library} PROPERTIES PREFIX ${CMAKE_BUILD_TYPE}/${prefixSV})
	endif()
	SET_TARGET_PROPERTIES(${library} PROPERTIES SUFFIX ${suffix})

    set("${library}_OUTPUT_BIN_SV" "${BromSock_BINARY_DIR}/${library}/${CMAKE_BUILD_TYPE}/${prefixSV}${library}${suffix}" PARENT_SCOPE)
    set("${library}_OUTPUT_BIN_CL" "${BromSock_BINARY_DIR}/${library}/${CMAKE_BUILD_TYPE}/${prefixCL}${library}${suffix}" PARENT_SCOPE)

	add_custom_command(
		TARGET ${output_target} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy
		"${BromSock_BINARY_DIR}/${library}/${CMAKE_BUILD_TYPE}/${prefixSV}${library}${suffix}"
		"${BromSock_BINARY_DIR}/${library}/${CMAKE_BUILD_TYPE}/${prefixCL}${library}${suffix}"
	)
endfunction()
