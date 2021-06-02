#include <string>
#include <nlohmann/json.hpp>
#include "opmonlib/OpmonService.hpp"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <sstream>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>
#include "JsonInfluxConverter.hpp"
#include <curl/curl.h>
#include "cpr/cpr.h"
#include <regex>

using json = nlohmann::json;

namespace dunedaq {

    ERS_DECLARE_ISSUE(influxopmon, CannotPostToDB,
        "Cannot post to Influx DB " << error,
        ((std::string)error))

    ERS_DECLARE_ISSUE(influxopmon, WrongURI,
        "Incorrect URI, use influxdb://<host>:<port>/<endpoint>?db=<mydb> instead of : " << fatal,
        ((std::string)fatal))
}


namespace dunedaq::influxopmon {

    class influxOpmonService : public dunedaq::opmonlib::OpmonService
    {
        public:


        explicit influxOpmonService(std::string uri) : dunedaq::opmonlib::OpmonService(uri) 
        {
            std::regex uri_re(R"(([a-zA-Z]+):\/\/([^:\/?#\s]+)+(?::(\d+))?(\/[^?#\s]+)?(?:\?(?:db=([^?#\s]+))))");

            std::smatch uri_match;
            if (!std::regex_match(uri, uri_match, uri_re)) 
            {
                ers::fatal(WrongURI(ERS_HERE, "Invalid URI syntax: " + uri));
                exit(0);
            }

            for (size_t i(0); i < uri_match.size(); ++i) 
            {
                    std::cout << i << "  " << uri_match[i] << std::endl;
            }

            m_host = uri_match[2];
            m_port = (!uri_match[3].str().empty() ? uri_match[3] : std::string("8086"));
            m_path = uri_match[4];
            m_dbname = uri_match[5];
        }

        void publish(nlohmann::json j)
        {
            jsonConverter.setInsertsVector(j);
            insertsVector = jsonConverter.getInsertsVector();  
            
            query = "";
            
            for (unsigned long int i = 0; i < insertsVector.size(); i++)
            {
                query = query + insertsVector[i] + "\n" ;
            }

	        executionCommand(m_host + ":" + m_port + m_path + "?db=" + m_dbname, query);
        }

        protected:
            typedef OpmonService inherited;
        private:
            std::string m_host;
            std::string m_port;
            std::string m_path;
            std::string m_dbname;

            
            std::vector<std::string> insertsVector;
            
            std::string query;
            const char* charPointer;
            influxopmon::JsonConverter jsonConverter;
        
        void executionCommand(std::string adress, std::string cmd) {

            cpr::Response response = cpr::Post(cpr::Url{adress}, cpr::Body{cmd});
            
            if (response.status_code >= 400) {
                ers::error(CannotPostToDB(ERS_HERE, "Error [" + std::to_string(response.status_code) + "] making request"));
            } else if (response.status_code == 0) {
                ers::error(CannotPostToDB(ERS_HERE, "Querry returned 0"));
            } 
        }

    };

}

extern "C" {
    std::shared_ptr<dunedaq::opmonlib::OpmonService> make(std::string service) {
        return std::shared_ptr<dunedaq::opmonlib::OpmonService>(new dunedaq::influxopmon::influxOpmonService(service));
    }
}

