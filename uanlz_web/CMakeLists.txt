cmake_minimum_required(VERSION 3.10)
include(GNUInstallDirs)

set(APP_NAME "uanlz_web")
project(${APP_NAME})
project(${PROJECT_NAME} LANGUAGES CXX VERSION ${SVERSION} DESCRIPTION "Unmanarc's Auditd Analyzer Framework - Web Server")

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
target_link_libraries(${APP_NAME} "-lmdz_xrpc_webserver")
target_link_libraries(${APP_NAME} "-lmdz_xrpc_common")
target_link_libraries(${APP_NAME} "-lmdz_proto_http")
target_link_libraries(${APP_NAME} "-lmdz_proto_mime")
target_link_libraries(${APP_NAME} "-lmdz_thr_threads")
target_link_libraries(${APP_NAME} "-lmdz_thr_safecontainers")
target_link_libraries(${APP_NAME} "-lmdz_net_sockets")
target_link_libraries(${APP_NAME} "-lmdz_prg_service")
target_link_libraries(${APP_NAME} "-lmdz_prg_logs")
target_link_libraries(${APP_NAME} "-lmdz_auth")
target_link_libraries(${APP_NAME} "-lmdz_auth_remote")
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


find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)
link_libraries(${JSONCPP_LIBRARIES})
target_include_directories(${APP_NAME} PUBLIC ${JSONCPP_INCLUDE_DIRS})
target_link_libraries(${APP_NAME} ${JSONCPP_LIBRARIES})

