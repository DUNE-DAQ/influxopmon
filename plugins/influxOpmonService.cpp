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

using json = nlohmann::json;

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
            int countArgs = 0;
            while (getline(ss, tmp, ':'))
            {
                ressource.push_back(tmp);
                countArgs++;
            }

            if (countArgs < 3)
            {
                std::cout << "invalid URI, follow: influx://proxyAdress:database:Delimiter:Tags(0..N) \n Example: influx://188.185.88.195:db1:.time=:.class_name=\n";
                exit(0);
            }

	    m_host = ressource[0];
            m_dbname = ressource[1];
            m_delimiter = ressource[2];

            for (unsigned long int i = 3; i < ressource.size(); i++)
            {
                tagSetVector.push_back(ressource[i]);
            }
	}

        void publish(nlohmann::json j)
        {
            
            jsonConverter.setInsertsVector(false, tagSetVector, m_delimiter, j.flatten().dump());
            insertsVector = jsonConverter.getInsertsVector();
            
            
            querry = "";
            
            for (unsigned long int i = 0; i < insertsVector.size(); i++)
            {
                querry = querry + insertsVector[i] + "\n" ;
            }

            //silent output
	    executionCommand(m_host + "/?db=" + m_dbname, querry);
	}

    protected:
        typedef OpmonService inherited;
    private:
        std::string m_host;
        std::string m_dbname;
        
        std::vector<std::string> tagSetVector;
        std::string m_delimiter;
        std::vector<std::string> insertsVector;
        
        std::string querry;
        const char* charPointer;
        influxopmon::JsonConverter jsonConverter;
	
	void executionCommand(std::string adress, std::string cmd) {

                cpr::Response response = cpr::Post(cpr::Url{adress}, cpr::Body{cmd});
        }

    };

}

extern "C" {
    std::shared_ptr<dunedaq::opmonlib::OpmonService> make(std::string service) {
        return std::shared_ptr<dunedaq::opmonlib::OpmonService>(new dunedaq::influxopmon::influxOpmonService(service));
    }
}

