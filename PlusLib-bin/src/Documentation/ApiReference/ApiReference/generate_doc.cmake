# ----------------------------------------------------------------------------
# Apply settings

set(DOXYGEN_CONFIG_FILE "C:/D/PBE/PlusLib-bin/src/Documentation/ApiReference/ApiReference/Doxyfile.txt")
set(UNCOMPRESSED_HELP_TARGET_DIR "C:/D/PBE/PlusLib-bin/src/Documentation/ApiReference/ApiReference")
set(CREATE_COMPRESSED_HELP "YES")
set(COMPRESSED_HELP_TARGET_DIR "C:/D/PBE/bin/Doc")
set(COMPRESSED_HELP_TARGET_FILE "PlusLib-ApiReference.chm")

# ----------------------------------------------------------------------------
# Generate documentation

execute_process(
  COMMAND "C:/Program Files/doxygen/bin/doxygen.exe" "${DOXYGEN_CONFIG_FILE}"
  WORKING_DIRECTORY ${UNCOMPRESSED_HELP_TARGET_DIR}
  RESULT_VARIABLE rv
  )

if(rv)
  message(FATAL_ERROR "error: Failed to generate documentation")
endif()

# ----------------------------------------------------------------------------
# Create the compressed help file (chm on Windows, tar.gz on other OS)

if(CREATE_COMPRESSED_HELP)
  if(WIN32)
    if(EXISTS "${UNCOMPRESSED_HELP_TARGET_DIR}/html/${COMPRESSED_HELP_TARGET_FILE}")
      file(COPY "${UNCOMPRESSED_HELP_TARGET_DIR}/html/${COMPRESSED_HELP_TARGET_FILE}" DESTINATION ${COMPRESSED_HELP_TARGET_DIR})
      message(STATUS "info: created '${COMPRESSED_HELP_TARGET_DIR}/${COMPRESSED_HELP_TARGET_FILE}'")
    else()
      message(STATUS "warning: could not create '${UNCOMPRESSED_HELP_TARGET_DIR}/html/${COMPRESSED_HELP_TARGET_FILE}'")
    endif()
  else()
    file(REMOVE "${COMPRESSED_HELP_TARGET_DIR}/${COMPRESSED_HELP_TARGET_FILE}")
    execute_process(COMMAND ${CMAKE_COMMAND} -E tar cfz ${COMPRESSED_HELP_TARGET_DIR}/${COMPRESSED_HELP_TARGET_FILE} html
      WORKING_DIRECTORY ${UNCOMPRESSED_HELP_TARGET_DIR}
      RESULT_VARIABLE rv
      )
    if(EXISTS "${COMPRESSED_HELP_TARGET_DIR}/${COMPRESSED_HELP_TARGET_FILE}")
      message(STATUS "info: created '${COMPRESSED_HELP_TARGET_DIR}/${COMPRESSED_HELP_TARGET_FILE}'")
    else()
      message(STATUS "warning: could not create '${COMPRESSED_HELP_TARGET_DIR}/${COMPRESSED_HELP_TARGET_FILE}'")
    endif()    
  endif()
endif()
