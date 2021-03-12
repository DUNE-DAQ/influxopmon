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

        std::string exec(const char* cmd) {
            std::array<char, 128> buffer;
            std::string result;
            std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
            if (!pipe) {
                throw std::runtime_error("popen() failed!");
            }
            while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
                result += buffer.data();
            }
            return result;
        }


        explicit influxOpmonService(std::string uri) : dunedaq::opmonlib::OpmonService(uri) {
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
            m_port = ressource[1];
            m_dbname = ressource[2];
            m_dbaccount = ressource[3];
            m_dbpassword = ressource[4];

            tagSetVector.push_back(".class_name=");
        }

        void publish(nlohmann::json j)
        {
            
            jsonConverter.setInsertsVector(false, tagSetVector, timeVariableName, j.flatten().dump());
            insertsVector = jsonConverter.getInsertsVector();
            
            
            querry = "curl -i -XPOST 'https://" + m_host + ":" + m_port + "/write?db=" + m_dbname + "' --header 'Authorization: Token " + m_dbaccount + ":" + m_dbpassword + "'  --data-binary '";
            
            for (unsigned long int i = 0; i < insertsVector.size(); i++)
            {
                std::cout << insertsVector[i];
                querry = querry + insertsVector[i] + "\n" ;
                
                
            }

            /*Test for bulk sending
            for (unsigned long int i = 0; i < 5; i++)
            {
                std::cout << insertsVector[0];
                querry = querry + insertsVector[0] + std::to_string(i) + "\n";


            }
            */

            querry = querry + "'";

            charPointer = querry.c_str();
            std::cout << exec(charPointer);
            
        }

    protected:
        typedef OpmonService inherited;
    private:
        std::string m_host;
        std::string m_port;
        std::string m_dbname;
        std::string m_dbaccount;
        std::string m_dbpassword;
        
        std::vector<std::string> tagSetVector;
        std::string timeVariableName = ".time=";
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

