cmake_minimum_required(VERSION 3.10)
include(GNUInstallDirs)

set(APP_NAME "uanlz_log2json")
project(${APP_NAME})
project(${PROJECT_NAME} LANGUAGES CXX VERSION ${SVERSION} DESCRIPTION "Unmanarc's Auditd Analyzer Framework - Log to JSON Translator")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

configure_file(src/config.h.in ${${APP_NAME}_SOURCE_DIR}/src/config.h)

file(GLOB_RECURSE EDV_INCLUDE_FILES "src/*.h*")
file(GLOB_RECURSE EDV_SOURCE_FILES "src/*.c*")

file(GLOB_RECURSE EDV_INCLUDE_FILES2 "src/*/*.h*")
file(GLOB_RECURSE EDV_SOURCE_FILES2 "src/*/*.c*")

add_executable( ${APP_NAME} ${EDV_INCLUDE_FILES} ${EDV_SOURCE_FILES} ${EDV_INCLUDE_FILES2} ${EDV_SOURCE_FILES2} )

install( TARGETS ${APP_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} )

if (EXTRAPREFIX)
    target_include_directories(${APP_NAME} PUBLIC ${EXTRAPREFIX}/include)
    target_link_libraries(${APP_NAME} "-L${EXTRAPREFIX}/lib")
    target_link_libraries(${APP_NAME} "-L${EXTRAPREFIX}/lib64")
endif()

if (CMAKE_INSTALL_PREFIX)
    target_include_directories(${APP_NAME} PUBLIC ${CMAKE_INSTALL_PREFIX}/include)
    target_include_directories(${APP_NAME} PUBLIC ${CMAKE_INSTALL_PREFIX}/usr/include)
    target_link_libraries(${APP_NAME} "-L${CMAKE_INSTALL_PREFIX}/lib")
    target_link_libraries(${APP_NAME} "-L${CMAKE_INSTALL_PREFIX}/lib64")
endif()


target_link_libraries(${APP_NAME} "-lpthread")

option(SSLRHEL7 "OpenSSL 1.1 For Red Hat 7.x provided by EPEL" OFF)
find_package(PkgConfig REQUIRED)
if (SSLRHEL7)
    pkg_check_modules(OPENSSL REQUIRED libssl11)
else()
    pkg_check_modules(OPENSSL REQUIRED libssl)
endif()
target_include_directories(${APP_NAME} PUBLIC ${OPENSSL_INCLUDE_DIRS})
target_compile_options(${APP_NAME} PUBLIC ${OPENSSL_CFLAGS_OTHER})
target_link_libraries(${APP_NAME} ${OPENSSL_STATIC_LDFLAGS})


if (WIN32)
    target_link_libraries(${APP_NAME} "-lws2_32 -liphlpapi")
elseif()
    target_link_libraries(${APP_NAME} "-lrt -ldl")
endif()

######################################

find_library (
    BOOST_REGEX_LIB
    NAMES boost_regex-mt-x32 libboost_regex-mt-x32 boost_regex-mt libboost_regex-mt boost_regex libboost_regex # what to look for
)
if (NOT BOOST_REGEX_LIB)
    message("ERROR: Did not found lib BOOST_REGEX_LIB")
else()
    message("-- BOOST_REGEX_LIB found at [${BOOST_REGEX_LIB}] for ${APP_NAME}")
    target_link_libraries(${APP_NAME} ${BOOST_REGEX_LIB})
endif()

find_library (
    BOOST_SYSTEM_LIB
    NAMES boost_system-mt-x32 libboost_system-mt-x32 boost_system-mt libboost_system-mt boost_system libboost_system # what to look for
)
if (NOT BOOST_SYSTEM_LIB)
    message("ERROR: Did not found lib BOOST_SYSTEM_LIB")
else()
    message("-- BOOST_SYSTEM_LIB found at [${BOOST_SYSTEM_LIB}] for ${APP_NAME}")
    target_link_libraries(${APP_NAME} ${BOOST_SYSTEM_LIB})
endif()


find_library (
    BOOST_THREAD_LIB
    NAMES boost_thread-mt-x32 libboost_thread-mt-x32 boost_thread-mt libboost_thread-mt boost_thread libboost_thread # what to look for
)
if (NOT BOOST_THREAD_LIB)
    message("ERROR: Did not found lib BOOST_THREAD_LIB")
else()
    message("-- BOOST_THREAD_LIB found at [${BOOST_THREAD_LIB}] for ${APP_NAME}")
    target_link_libraries(${APP_NAME} ${BOOST_THREAD_LIB})
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)
link_libraries(${JSONCPP_LIBRARIES})
target_include_directories(${APP_NAME} PUBLIC ${JSONCPP_INCLUDE_DIRS})
target_link_libraries(${APP_NAME} ${JSONCPP_LIBRARIES})



################################################################################
# libMantids packages:
pkg_check_modules(MANTIDS_LIBS REQUIRED
    mdz_xrpc_fast
    mdz_thr_threads
    mdz_thr_safecontainers
    mdz_proto_linerecv
    mdz_net_sockets
    mdz_prg_service
    mdz_prg_logs
    mdz_thr_mutex
    mdz_mem_vars
    mdz_hlp_functions
)
include_directories(${MANTIDS_LIBS_INCLUDE_DIRS})
link_directories(${MANTIDS_LIBS_LIBRARY_DIRS})
target_link_libraries(${APP_NAME} ${MANTIDS_LIBS_LIBRARIES})
################################################################################
