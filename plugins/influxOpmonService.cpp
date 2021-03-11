#include <string>
#include <nlohmann/json.hpp>

#include "opmonlib/OpmonService.hpp"
#include "influxdb.hpp"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <sstream>

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

            std::cout << "Hello world \n";

            uri = uri.substr(uri.find("/") + 2);
            std::string tmp;
            std::stringstream ss(uri);
            std::vector<std::string> ressource;
            while (getline(ss, tmp, ':'))
            {
                ressource.push_back(tmp);
            }

            //std::string uri = "influx://dbod-testinfluxyd.cern.ch:8095:db1:admin:admin"
            m_host = ressource[0];
            m_port = stoi(ressource[1]);
            m_dbname = ressource[2];
            m_dbaccount = ressource[3];
            m_dbpassword = ressource[4];

            std::cout << m_host+ " \n";

        }

        void publish(nlohmann::json j)
        {

            // influxdb_cpp::server_info si(m_host, m_port, .....);
        // FIXME: do here the reformatting of j and the posting to the db

        }
    protected:
        typedef OpmonService inherited;
    private:
        std::string m_host;
        int32_t m_port;
        std::string m_dbname;
        std::string m_dbaccount;
        std::string m_dbpassword;
        // FIXME: add here utility methods

    };

}

extern "C" {
    std::shared_ptr<dunedaq::opmonlib::OpmonService> make(std::string service) {
        return std::shared_ptr<dunedaq::opmonlib::OpmonService>(new dunedaq::influxopmon::influxOpmonService(service));
    }
}

