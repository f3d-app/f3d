function(f3d_python_windows_dll_fixup)
	cmake_parse_arguments(F3D_DLL_FIXUP "" "OUTPUT" "RELATIVE_PATHS;ABSOLUTE_PATHS" ${ARGN})

	if(WIN32)
		set(_fixup [=[
if sys.version_info >= (3, 8) and sys.platform == 'win32':
    from pathlib import Path
    import os
]=]
		)

		foreach(_path ${F3D_DLL_FIXUP_ABSOLUTE_PATHS})
			set(_fixup "${_fixup}    os.add_dll_directory('${_path}')\n")
		endforeach()

		foreach(_path ${F3D_DLL_FIXUP_RELATIVE_PATHS})
			set(_fixup "${_fixup}    os.add_dll_directory((Path(__file__).parent / '${_path}').resolve())\n")
		endforeach()

		set(${F3D_DLL_FIXUP_OUTPUT} ${_fixup} PARENT_SCOPE)
	else()
		set(${F3D_DLL_FIXUP_OUTPUT} "" PARENT_SCOPE)
	endif()
endfunction()
