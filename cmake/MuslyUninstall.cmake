
set(INSTALL_MANIFEST "${CMAKE_CURRENT_BINARY_DIR}/install_manifest.txt")
if(EXISTS ${INSTALL_MANIFEST})
    message(STATUS "Uninstalling Musly ...")

    file(STRINGS ${INSTALL_MANIFEST} installed_files)
    foreach(file ${installed_files})
        if(EXISTS ${file})
            message(STATUS "Deleting file: '${file}'")

            execute_process(
                COMMAND ${CMAKE_COMMAND} -E remove ${file}
                OUTPUT_VARIABLE rm_out
                RESULT_VARIABLE rm_retval
            )
            if (NOT "${rm_retval}" STREQUAL 0)
                message(FATAL_ERROR "Failed to remove file '${file}'.")
            endif()
        else()
            message(STATUS "Expected '${file}' to exist, but it does not.")
        endif()
    endforeach()

    message(STATUS "... done.")
else()
    message(STATUS "no install manifest at '${INSTALL_MANIFEST}' found. Maybe the project has not been installed yet?")
endif()
