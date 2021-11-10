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
LIBS += -L$$PREFIX/lib -L$$PREFIX/lib64 -L$$OSSLIBS_PREFIX/lib -L$$OSSLIBS_PREFIX/lib64

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
LIBS += -lcx2_net_sockets -lssl -lcrypto
LIBS += -lcx2_netp_linerecv
LIBS += -lcx2_prg_service -lcx2_prg_logs
LIBS += -lcx2_thr_mutex
LIBS += -lcx2_mem_vars -lcx2_hlp_functions
LIBS += -lpthread
LIBS += -ljsoncpp
LIBS += -lboost_system -lboost_regex -lboost_thread

SOURCES += \
        src/events/audit_class.cpp \
        src/events/events_manager.cpp \
        src/input/inputs.cpp \
        src/events/events_distributionthreads.cpp \
        src/output/output_base.cpp \
        src/output/output_jsontcp.cpp \
        src/output/outputs.cpp \
        src/events/audit_event.cpp \
        src/events/audit_host.cpp \
        src/globals.cpp \
        src/main.cpp \
        src/input/tcpserver.cpp \
        src/input/tcplineprocessor.cpp \
        src/rpcimpl.cpp \
        src/vars/namedefs.cpp \
        src/events/audit_var.cpp

HEADERS += \
    src/events/audit_class.h \
    src/events/events_manager.h \
    src/input/inputs.h \
    src/events/events_distributionthreads.h \
    src/output/output_base.h \
    src/output/output_jsontcp.h \
    src/output/outputs.h \
    src/events/audit_event.h \
    src/events/audit_host.h \
    src/globals.h \
    src/input/tcpserver.h \
    src/input/tcplineprocessor.h \
    src/rpcimpl.h \
    src/vars/namedefs.h \
    src/events/audit_var.h
