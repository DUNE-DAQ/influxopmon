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
        "Cannot post to Influx DB " << error,
        ((std::string)error))
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

            std::cout << "Arguments count: "<< std::to_string(countArgs) << "\n";

            if (countArgs == 3)
            {
                dbPort == ressource[2] + "/write";
            }
            else if (countArgs != 2)
            {
                std::cout << "invalid URI, follow : influx://proxyAdress:database:(optional db port):(optional db user name):(optional db password) \n Example: influx://188.185.88.195:db1\n";
                exit(0);
            }

	        hostName = ressource[0];
            dbName = ressource[1];



	}

        void publish(nlohmann::json j)
	{
            jsonConverter.setInsertsVector(j);
            insertsVector = jsonConverter.getInsertsVector();  
            
            querry = "";
            
            for (unsigned long int i = 0; i < insertsVector.size(); i++)
            {
                querry = querry + insertsVector[i] + "\n" ;
            }

	        executionCommand(hostName + ":80/insert?db=db1", querry);
	}

    protected:
        typedef OpmonService inherited;
    private:
        std::string hostName;
        std::string dbName;
        std::string dbPort = "";

        
        std::vector<std::string> insertsVector;
        
        std::string querry;
        const char* charPointer;
        influxopmon::JsonConverter jsonConverter;
	
	void executionCommand(std::string adress, std::string cmd) {

        cpr::Response response = cpr::Post(cpr::Url{adress}, cpr::Body{cmd});
		std::cout << adress << "\n";	
		std::cout << cmd << "\n";	
		if (response.status_code >= 400) {
			ers::error(CannotPostToDB(ERS_HERE, "Error [" + std::to_string(response.status_code) + "] making request"));
		} else if (response.status_code == 0) {
			ers::warning(CannotPostToDB(ERS_HERE, "Querry returned 0"));
		} 
	}

    };

}

extern "C" {
    std::shared_ptr<dunedaq::opmonlib::OpmonService> make(std::string service) {
        return std::shared_ptr<dunedaq::opmonlib::OpmonService>(new dunedaq::influxopmon::influxOpmonService(service));
    }
}

