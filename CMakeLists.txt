cmake_minimum_required(VERSION 3.12)
project(influxopmon VERSION 1.1.0)

find_package(daq-cmake REQUIRED)

daq_setup_environment()

find_package(opmonlib REQUIRED)
find_package(CPR)

FIND_PACKAGE(CURL)

INCLUDE_DIRECTORIES(${CURL_INCLUDE_DIR})
SET(requiredlibs ${requiredlibs} ${CURL_LIBRARIES} )



##############################################################################
# Main library

#daq_add_library(*.cpp LINK_LIBRARIES ${INFLUXOPMON_DEPENDENCIES})

##############################################################################
# Plugins

daq_add_plugin(influxOpmonService duneOpmonService LINK_LIBRARIES opmonlib::opmonlib CURL::libcurl  ${CPR_LIBRARIES})
target_include_directories(${PROJECT_NAME}_influxOpmonService_duneOpmonService PUBLIC ${CPR_INCLUDE_DIRS})

##############################################################################
# Integration tests

#daq_add_application(opmonlib_test opmonlib_test.cpp TEST LINK_LIBRARIES opmonlib)

##############################################################################
# No unit tests written

##############################################################################

daq_install()

