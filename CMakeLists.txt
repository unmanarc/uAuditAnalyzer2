cmake_minimum_required(VERSION 3.10)

set(SVERSION 2.1.1)
project(uAuditAnalyzer2)
project(${PROJECT_NAME} VERSION ${SVERSION} DESCRIPTION "Unmanarc's Audit Analyzer")

ADD_SUBDIRECTORY(uanlz_alert)
ADD_SUBDIRECTORY(uanlz_log2json)
ADD_SUBDIRECTORY(uanlz_web)
