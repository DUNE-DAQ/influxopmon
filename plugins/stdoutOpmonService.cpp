#include <iostream>
#include <string>
#include <nlohmann/json.hpp>

#include "opmonlib/OpmonService.hpp"

namespace dunedaq::opmonlib {
  
  class stdoutOpmonService : public OpmonService
  {
  public: 
    explicit stdoutOpmonService(std::string uri) : OpmonService(uri) {
        auto sep = uri.find("://");
        m_style="flat";
        if (sep != std::string::npos) { // assume filename
          m_style = uri.substr(sep+3);
        }
    }

    void publish( nlohmann::json j )
    {
      if(m_style == "flat") {
         std::cout << std::setw(4) << j.flatten() << '\n';
      } 
      else if (m_style == "formatted") { 
         std::cout << j.dump(2) << std::endl;
      }
      else {
         std::cout << j.dump() << std::endl;
      }
    }

  protected:
    typedef OpmonService inherited;
  private:
    std::string m_style;
};

}

extern "C" {
  std::shared_ptr<dunedaq::opmonlib::OpmonService> make(std::string service) {
    return std::shared_ptr<dunedaq::opmonlib::OpmonService>(new dunedaq::opmonlib::stdoutOpmonService(service));
  }
}

