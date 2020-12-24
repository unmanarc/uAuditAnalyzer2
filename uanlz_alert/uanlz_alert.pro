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
LIBS += -lcx2_thr_threads -lcx2_thr_safecontainers
LIBS += -lcx2_netp_linerecv
LIBS += -lcx2_net_sockets -lssl -lcrypto
LIBS += -lcx2_scripts_jsonexpreval -lboost_regex
LIBS += -lcx2_prg_service -lcx2_prg_logs
LIBS += -lcx2_thr_mutex
LIBS += -lcx2_mem_vars -lcx2_hlp_functions
LIBS += -lpthread
LIBS += -ljsoncpp

SOURCES +=  \
    src/globals.cpp \
    src/input/inputs.cpp \
    src/input/tcplineprocessor.cpp \
    src/input/tcpserver.cpp \
    src/main.cpp \
    src/rpcimpl.cpp \
    src/rules/rules.cpp
HEADERS +=  \
    src/globals.h \
    src/input/inputs.h \
    src/input/tcplineprocessor.h \
    src/input/tcpserver.h \
    src/rpcimpl.h \
    src/rules/rules.h
