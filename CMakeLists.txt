cmake_minimum_required(VERSION 3.12)
project(influxopmon VERSION 1.0.1)

find_package(daq-cmake REQUIRED)

daq_setup_environment()

find_package(opmonlib REQUIRED)


##############################################################################
# Main library

#daq_add_library(*.cpp LINK_LIBRARIES ${INFLUXOPMON_DEPENDENCIES})

##############################################################################
# Plugins

daq_add_plugin(influxOpmonService duneOpmonService LINK_LIBRARIES opmonlib::opmonlib)

##############################################################################
# Integration tests

#daq_add_application(opmonlib_test opmonlib_test.cpp TEST LINK_LIBRARIES opmonlib)

##############################################################################
# No unit tests written

##############################################################################

daq_install()

