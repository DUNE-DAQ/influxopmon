#include "opmonlib/OpmonService.hpp"
#include <array>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include "JsonInfluxConverter.hpp"
// #include <curl/curl.h>
#include "cpr/cpr.h"

using json = nlohmann::json;

namespace dunedaq {

    ERS_DECLARE_ISSUE(influxopmon, CannotPostToDB,
        "Cannot post to Influx DB " << name,
        ((std::string)name))

}


namespace dunedaq::influxopmon {

class influxOpmonService : public dunedaq::opmonlib::OpmonService {
    public:

        explicit influxOpmonService(std::string uri) : dunedaq::opmonlib::OpmonService(uri) {

          /*
            "([a-zA-Z]):\/\/([^:\/?#\s])+(?::(\d))?(\/[^?#\s])?(?:\?([^#\s]))?(#[^\s])?"

            * 1st Capturing Group `([a-zA-Z])`: Matches protocol
            * 2nd Capturing Group `([^:\/?#\s])+`: Matches hostname
            * 3rd Capturing Group `(\d)`: Matches port, optional
            * 4th Capturing Group `(\/[^?#\s])?`: Matches endpoint/path
            * 5th Capturing Group `([^#\s])`: Matches query
            * 6th Capturing Group `([^\s])?`: Matches fragment

            simplified version
            "([a-zA-Z]+):\/\/([^:\/?#\s]+)+(?::(\d+))?(\/[^?#\s]+)?(?:\?(?:db=([^?#\s]+)))"
            * 1st Capturing Group `([a-zA-Z])`: Matches protocol
            * 2nd Capturing Group `([^:\/?#\s])+`: Matches hostname
            * 3rd Capturing Group `(\d)`: Matches port, optional
            * 4th Capturing Group `(\/[^?#\s])?`: Matches endpoint/path
            * 5th Capturing Group `([^#\s])`: Matches dbname

            std::regex
            uri_re(R"(([a-zA-Z]+):\/\/([^:\/?#\s]+)+(?::(\d+))?(\/[^?#\s]+)?(?:\?([^#\s]+))?(?:(#[^\s]+))?)");
            */
          std::regex uri_re(R"(([a-zA-Z]+):\/\/([^:\/?#\s]+)+(?::(\d+))?(\/[^?#\s]+)?(?:\?(?:db=([^?#\s]+))))");

          std::smatch uri_match;
          if (!std::regex_match(uri, uri_match, uri_re)) {
            // Error here
            std::cerr << "Invalid URI syntax: " << uri << std::endl;
            }

            for (size_t i(0); i < uri_match.size(); ++i) {
                std::cout << i << "  " << uri_match[i] << std::endl;
            }

            m_host = uri_match[2];
            m_port = (!uri_match[3].str().empty() ? uri_match[3] : std::string("8086"));
            m_path = uri_match[4];
            m_dbname = uri_match[5];

            // uri.substr(uri.find("/") + 2);
            // std::string tmp;
            // std::stringstream ss(uri);
            // std::vector<std::string> ressource;
            // int countArgs = 0;
            // while (getline(ss, tmp, ':')) {
            //     ressource.push_back(tmp);
            //     countArgs++;
            // }

            // if (countArgs < 1)
            // {
            //     std::cout << "invalid URI, follow: influx://proxyAdress:database:Delimiter:Tags(0..N) \n Example: influx://188.185.88.195:db1\n";
            //     exit(0);
            // }

            // m_host = ressource[0];
            // m_port = ressource[1];
            // m_dbname = ressource[2];

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

            //silent output
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

      void executionCommand(std::string adress, std::string cmd)
      {

        std::cout << "Address: " << adress << std::endl;
        std::cout << "Body: " << cmd << std::endl;
        cpr::Response response = cpr::Post(cpr::Url{ adress }, cpr::Body{ cmd });
        // std::cout << cmd << "\n";
        if (response.status_code >= 400) {
          std::cerr << "Error [" << response.status_code << "] making request" << std::endl;
        } else if (response.status_code == 0) {
          std::cout << "Status code " << response.status_code << std::endl;
          std::cout << "Empty query" << std::endl;
        } 
	}

    };

}

extern "C" {
    std::shared_ptr<dunedaq::opmonlib::OpmonService> make(std::string service) {
        return std::shared_ptr<dunedaq::opmonlib::OpmonService>(new dunedaq::influxopmon::influxOpmonService(service));
    }
}

