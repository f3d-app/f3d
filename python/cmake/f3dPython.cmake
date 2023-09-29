# On Windows, usage of PATH to find the DLL is not possible anymore
# see https://stackoverflow.com/a/64303856/2609654
# This function should be called to generate F3D_ABSOLUTE_DLLS_FIXUP and F3D_RELATIVE_DLLS_FIXUP
# CMake variables used to fill the "blessed" paths for Windows
function(f3d_python_windows_dll_fixup)
	cmake_parse_arguments(F3D_DLL_FIXUP "" "OUTPUT" "PATHS" ${ARGN})

	set(_fixup "generated at build\n")
	foreach(_path ${F3D_DLL_FIXUP_PATHS})
		set(_fixup "${_fixup}    '${_path}',\n")
	endforeach()
	set(${F3D_DLL_FIXUP_OUTPUT} ${_fixup} PARENT_SCOPE)
endfunction()
