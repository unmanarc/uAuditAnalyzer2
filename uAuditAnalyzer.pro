TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

isEmpty(PREFIX) {
    PREFIX = /usr/local
}
# includes dir
LIBS += -L$$PREFIX/lib

QMAKE_INCDIR += $$PREFIX/include
INCLUDEPATH += $$PREFIX/include

QMAKE_INCDIR += src
INCLUDEPATH += src

#Target directory
DESTDIR=bin
#Intermediate object files directory
OBJECTS_DIR=obj

# INSTALLATION:
target.path = $$PREFIX/bin
INSTALLS += target

LIBS+= -lPocoFoundation -lPocoUtil -lpqxx
LIBS+= -lcx_net_sockets -lcx_mem_streams -lcx_net_threadedacceptor -lcx_mem_streamparser -lcx_protocols_linerecv  -lcx_mem_containers -lcx_mem_abstracts -lcx_scripts_jsonexpreval
LIBS += -lpthread -lcx_thr_mutex
LIBS += -ljsoncpp
LIBS += -lboost_system -lboost_regex -lboost_thread

SOURCES += \
        src/events/audit_class.cpp \
        src/output/output_base.cpp \
        src/output/output_jsontcp.cpp \
        src/output/output_postgresql.cpp \
        src/output/processorthreads_output.cpp \
        src/events/audit_event.cpp \
        src/events/audit_host.cpp \
        src/events/eventsmanager.cpp \
        src/globals.cpp \
        src/main.cpp \
        src/rawlogs/rawlogs_host.cpp \
        src/rawlogs/rawlogs_manager.cpp \
        src/rules/rules.cpp \
        src/serverapp.cpp \
        src/input/tcplineprocessor.cpp \
        src/vars/namedefs.cpp \
        src/events/audit_var.cpp

HEADERS += \
    src/events/audit_class.h \
    src/output/output_base.h \
    src/output/output_jsontcp.h \
    src/output/output_postgresql.h \
    src/output/processorthreads_output.h \
    src/events/audit_event.h \
    src/events/audit_host.h \
    src/events/eventsmanager.h \
    src/globals.h \
    src/globals_ext.h \
    src/rawlogs/rawlogs_host.h \
    src/rawlogs/rawlogs_manager.h \
    src/rules/rules.h \
    src/serverapp.h \
    src/input/tcplineprocessor.h \
    src/vars/namedefs.h \
    src/events/audit_var.h
