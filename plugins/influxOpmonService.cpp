#include <string>
#include <nlohmann/json.hpp>

#include "opmonlib/OpmonService.hpp"
#include "influxdb.hpp"

namespace dunedaq {

     ERS_DECLARE_ISSUE(influxopmon, CannotPostToDB,
                     "Cannot post to Influx DB " << name,
                      ((std::string)name))

}


namespace dunedaq::influxopmon {

class influxOpmonService : public dunedaq::opmonlib::OpmonService
{
  public: 
    explicit influxOpmonService(std::string uri) : dunedaq::opmonlib::OpmonService(uri) {
        // FIXME: Get the DB connection string from the URI.
        // The URI will be something like influx://db_host:db_port:db_name....
        m_host = "dbod-testinfluxyd.cern.ch";
        m_port = 8095;
        m_dbname = "db1";

        std::cout << "Hello world";
    }

    void publish( nlohmann::json j )
    {

        // influxdb_cpp::server_info si(m_host, m_port, .....);
	// FIXME: do here the reformatting of j and the posting to the db

    }
  protected:
    typedef OpmonService inherited;
  private:
    //std::ofstream m_ofs;
    std::string m_host;
    int32_t m_port;
    std::string m_dbname;
    // FIXME: add here utility methods
    
};

}

extern "C" {
  std::shared_ptr<dunedaq::opmonlib::OpmonService> make(std::string service) {
    return std::shared_ptr<dunedaq::opmonlib::OpmonService>(new dunedaq::influxopmon::influxOpmonService(service));
  }
}

