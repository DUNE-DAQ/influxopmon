#include <iostream>
#include <string>
#include <atomic>
#include <chrono>
#include <future>
#include <thread>

#include "opmonlib/MetricRegistry.hpp"
#include "opmonlib/MetricMonitor.hpp"

using namespace dunedaq::opmonlib;

int main(int /*argc*/, char** /*argv*/)
{
  std::atomic<float> myMetric(0.1);
  std::atomic<int> myMetric_int(5);
  std::atomic<double> myMetric_double(500);
  std::atomic<size_t> myMetric_sizet(40960);
   
  MetricMonitor mmonitor(1, 1, "HostName",  "AppName");

  std::map<std::string, std::string> parameters;
  parameters.insert({"fileName", "metric_output.json"});
  mmonitor.setupPublisher("file", parameters);

  MetricRegistry mregistry = MetricRegistry::getInstance();
  mregistry.registerMetric<std::atomic<float>>("Temperature", std::ref(myMetric));
  mregistry.getValueOfMetric<std::atomic<float>>("Temperature");
  mregistry.registerMetric<std::atomic<int>>("Humidity", std::ref(myMetric_int));
  mregistry.getValueOfMetric<std::atomic<int>>("Humidity");
  
  // 5 seconds later, I'll modify myMetrics:
  for (unsigned i=0; i < 10; ++i) {
    myMetric = myMetric + 1.1;
    myMetric_int=myMetric_int + 1;
    myMetric_double=myMetric_double + 1;
    myMetric_sizet=myMetric_sizet+1;
    //std::this_thread::sleep_for(5s);
  }

  mregistry.getValueOfMetric<std::atomic<float>>("Temperature");
  mregistry.getValueOfMetric<std::atomic<int>>("Humidity");
  mmonitor.monitor(mregistry.getMetrics());
}                                                                    
