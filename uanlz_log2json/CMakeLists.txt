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

target_link_libraries(${APP_NAME} "-lmdz_xrpc_fast")
target_link_libraries(${APP_NAME} "-lmdz_thr_threads")
target_link_libraries(${APP_NAME} "-lmdz_thr_safecontainers")
target_link_libraries(${APP_NAME} "-lmdz_proto_linerecv")
target_link_libraries(${APP_NAME} "-lmdz_net_sockets")
target_link_libraries(${APP_NAME} "-lmdz_prg_service")
target_link_libraries(${APP_NAME} "-lmdz_prg_logs")
target_link_libraries(${APP_NAME} "-lmdz_thr_mutex")
target_link_libraries(${APP_NAME} "-lmdz_mem_vars")
target_link_libraries(${APP_NAME} "-lmdz_hlp_functions")

target_link_libraries(${APP_NAME} "-lpthread")
target_link_libraries(${APP_NAME} "-lssl")
target_link_libraries(${APP_NAME} "-lcrypto")

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

