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
#include "CommandExecution.hpp"



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
            while (getline(ss, tmp, ':'))
            {
                ressource.push_back(tmp);
            }

            //std::string uri = "influx://dbod-testinfluxyd.cern.ch:8095:db1:admin:admin:https:.time=:.class_name="
            m_host = ressource[0];
            m_port = ressource[1];
            m_dbname = ressource[2];
            m_dbaccount = ressource[3];
            m_dbpassword = ressource[4];
            m_postProtocol = ressource[5];
            m_delimiter = ressource[6];

            for (unsigned long int i = 7; i < ressource.size(); i++)
            {
                tagSetVector.push_back(ressource[i]);
            }
          
            
        }

        void publish(nlohmann::json j)
        {
            
            jsonConverter.setInsertsVector(false, tagSetVector, m_delimiter, j.flatten().dump());
            insertsVector = jsonConverter.getInsertsVector();
            
            
            querry = "curl -i -XPOST '"+ m_postProtocol +"://" + m_host + ":" + m_port + "/write?db=" + m_dbname + "' --header 'Authorization: Token " + m_dbaccount + ":" + m_dbpassword + "'  --data-binary '";
            
            for (unsigned long int i = 0; i < insertsVector.size(); i++)
            {
                std::cout << insertsVector[i];
                querry = querry + insertsVector[i] + "\n" ;              
            }
            querry = querry + "'";

            charPointer = querry.c_str();
            std::cout << jsonConverter.getSetInsertsVector(charPointer);
        }

    protected:
        typedef OpmonService inherited;
    private:
        std::string m_host;
        std::string m_port;
        std::string m_dbname;
        std::string m_dbaccount;
        std::string m_dbpassword;
        std::string m_postProtocol;
        
        std::vector<std::string> tagSetVector;
        std::string m_delimiter;
        std::vector<std::string> insertsVector;
        
        std::string querry;
        const char* charPointer;
        JsonConverter jsonConverter;

    };

}

extern "C" {
    std::shared_ptr<dunedaq::opmonlib::OpmonService> make(std::string service) {
        return std::shared_ptr<dunedaq::opmonlib::OpmonService>(new dunedaq::influxopmon::influxOpmonService(service));
    }
}

