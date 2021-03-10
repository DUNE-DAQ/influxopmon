#ifndef OPMONLIB_INCLUDE_OPMONLIB_INFOMANAGER_HPP_
#define OPMONLIB_INCLUDE_OPMONLIB_INFOMANAGER_HPP_

#include <nlohmann/json.hpp>
#include "opmonlib/InfoProvider.hpp"
#include "opmonlib/OpmonService.hpp"
#include <atomic>
#include <thread>

namespace dunedaq::opmonlib {

  class InfoManager 
  {
    public:
      InfoManager( std::string service ); // Constructor
      InfoManager( dunedaq::opmonlib::OpmonService& service );
      void publish_info( int level );
      nlohmann::json gather_info( int level);
      void set_provider( opmonlib::InfoProvider& p );
      void start(uint32_t interval_sec, uint32_t level);
      void stop();
    
    private:
      void run(uint32_t interval_sec, uint32_t level);
 
      mutable opmonlib::InfoProvider *m_ip = nullptr;
      std::shared_ptr<opmonlib::OpmonService> m_service;
      std::atomic<bool> m_running; 
      std::thread m_thread;

  };

}

#endif
