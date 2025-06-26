cmake_minimum_required(VERSION 3.10)

set(SVERSION 2.4.0)
project(uAuditAnalyzer2)
project(${PROJECT_NAME} VERSION ${SVERSION} DESCRIPTION "Unmanarc's Audit Analyzer")

ADD_SUBDIRECTORY(uanlz_alert)
ADD_SUBDIRECTORY(uanlz_log2json)
ADD_SUBDIRECTORY(uanlz_web)



#############################################################################################################################
# Update version in spec.:

# File where %define version is located
set(SPEC_FILE "${CMAKE_SOURCE_DIR}/rpm/build.spec")

# Create a temporary file
file(WRITE ${CMAKE_BINARY_DIR}/temp_build_spec.txt "")

# Read the original file line by line
file(STRINGS ${SPEC_FILE} SPEC_LINES)

foreach(LINE ${SPEC_LINES})
    # Find the line containing %define version and replace it with SVERSION value
    if("${LINE}" MATCHES "^%define version .*$")
        file(APPEND ${CMAKE_BINARY_DIR}/temp_build_spec.txt "%define version ${SVERSION}\n")
    else()
        file(APPEND ${CMAKE_BINARY_DIR}/temp_build_spec.txt "${LINE}\n")
    endif()
endforeach()

# Replace the original file with the temporary one
file(RENAME ${CMAKE_BINARY_DIR}/temp_build_spec.txt ${SPEC_FILE})
