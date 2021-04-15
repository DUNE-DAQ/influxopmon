cmake_minimum_required(VERSION 3.12)
project(influxopmon VERSION 1.1.0)

find_package(daq-cmake REQUIRED)

daq_setup_environment()

find_package(opmonlib REQUIRED)



#include(FetchContent)
#FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/whoshuu/cpr.git GIT_TAG c8d33915dbd88ad6c92b258869b03aba06587ff9) # the commit hash for 1.5.0
#FetchContent_MakeAvailable(cpr)

FIND_PACKAGE(CURL)

INCLUDE_DIRECTORIES(${CURL_INCLUDE_DIR})
SET(requiredlibs ${requiredlibs} ${CURL_LIBRARIES} )




##############################################################################
# Main library

#daq_add_library(*.cpp LINK_LIBRARIES ${INFLUXOPMON_DEPENDENCIES} cpr::cpr)

##############################################################################
# Plugins

daq_add_plugin(influxOpmonService duneOpmonService LINK_LIBRARIES opmonlib::opmonlib CURL::libcurl)


#target_link_libraries(influxOpmonService PRIVATE cpr::cpr)


##############################################################################
# Integration tests

#daq_add_application(opmonlib_test opmonlib_test.cpp TEST LINK_LIBRARIES opmonlib)

##############################################################################
# No unit tests written

##############################################################################

daq_install()

