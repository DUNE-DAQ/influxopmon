cmake_minimum_required(VERSION 3.12)
project(influxopmon VERSION 1.4.0)

find_package(daq-cmake REQUIRED)

daq_setup_environment()

find_package(opmonlib REQUIRED)
find_package(CPR)
find_package(CURL)

##############################################################################
# Plugins

daq_add_plugin(influxOpmonService duneOpmonService LINK_LIBRARIES opmonlib::opmonlib CURL::libcurl  ${CPR_LIBRARIES})
target_include_directories(${PROJECT_NAME}_influxOpmonService_duneOpmonService PUBLIC ${CPR_INCLUDE_DIRS})

##############################################################################
# No integration tests written

##############################################################################
# No unit tests written

##############################################################################

daq_install()

