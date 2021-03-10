#include <string>
#include <vector>
#include <nlohmann/json.hpp>

#include "opmonlib/OpmonService.hpp"
#include "JsonInfluxConverter/JsonInfluxConverter.h"
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
        uri = uri.substr(uri.find("/") + 2);
        std::string tmp;
        std::stringstream ss(uri);
        std::vector<std::string> ressource;
        while (getline(ss, tmp, ':'))
        {
            ressource.push_back(tmp);
        }

        // FIXME: Get the DB connection string from the URI.
        // The URI will be something like influx://db_host:db_port:db_name....
        m_host = ressource[0];
        m_port = stoi(ressource[1]);
        m_dbname = ressource[2];
        m_dbaccount = ressource[3];
        m_dbpassword = ressource[4];

        //std::cout << "contacting" + host m_host;
    }

    void publish( nlohmann::json j )
    {

        // influxdb_cpp::server_info si(m_host, m_port, .....);
        tagSetVector.push_back(".class_name=");

        influxdb_cpp::server_info si(m_host, m_port, m_dbname, m_dbaccount, m_dbpassword);
        //influxdb_cpp::server_info si("dbod-testinfluxyd.cern.ch", 8095, "pyexample", "admin", "admin");
        std::string resp;

        jsonConverter.setInsertsVector(false, tagSetVector, timeVariableName, j);
        std::vector<std::string> insertsVector = jsonConverter.getInsertsVector();

        for (int i = 0; i < insertsVector.size(); i++)
        {
            influxdb_cpp::query(resp, insertsVector[i], si);
            std::cout << resp << std::endl;
        }
    }
  protected:
    typedef OpmonService inherited;
  private:
    //std::ofstream m_ofs;
    std::string m_host;
    int32_t m_port;
    std::string m_dbname;
    std::string m_dbaccount;
    std::string m_dbpassword;
    
    std::vector<std::string> tagSetVector;
    std::string timeVariableName = ".time=";
    JsonConverter jsonConverter;
};

}

extern "C" {
  std::shared_ptr<dunedaq::opmonlib::OpmonService> make(std::string service) {
    return std::shared_ptr<dunedaq::opmonlib::OpmonService>(new dunedaq::influxopmon::influxOpmonService(service));
  }
}

