TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

isEmpty(PREFIX) {
    PREFIX = /usr/local
}

isEmpty(OSSLIBS_PREFIX) {
    OSSLIBS_PREFIX = /opt/osslibs
}

# includes dir
LIBS += -L$$PREFIX/lib -L$$OSSLIBS_PREFIX/lib

QMAKE_INCDIR += ..
INCLUDEPATH += ..

QMAKE_INCDIR += src
INCLUDEPATH += src

QMAKE_INCDIR += $$PREFIX/include
INCLUDEPATH += $$PREFIX/include

QMAKE_INCDIR += $$OSSLIBS_PREFIX/include
INCLUDEPATH += $$OSSLIBS_PREFIX/include

include(../compiler.pri)

#Target directory
DESTDIR=bin
#Intermediate object files directory
OBJECTS_DIR=obj

# INSTALLATION:
target.path = $$PREFIX/bin
INSTALLS += target


LIBS += -lcx2_xrpc_fast
LIBS += -lcx2_xrpc_webserver -lcx2_xrpc_common -lcx2_netp_http -lcx2_netp_mime
LIBS += -lcx2_thr_mutex -lcx2_thr_safecontainers -lcx2_thr_threads
LIBS += -lcx2_prg_service -lcx2_prg_logs
LIBS += -lcx2_auth -lcx2_auth_db -lcx2_db_sqlite3 -lcx2_db -lsqlite3
LIBS += -lcx2_hlp_functions
LIBS += -lcx2_net_sockets
LIBS += -lcx2_mem_vars

LIBS += -lboost_regex -lpthread -ljsoncpp  -lssl -lcrypto

SOURCES +=  \
    src/globals.cpp \
    src/main.cpp \
    src/rpcserverimpl.cpp \
    src/webserverimpl.cpp
HEADERS +=  \
    src/globals.h \
    src/rpcserverimpl.h \
    src/webserverimpl.h
